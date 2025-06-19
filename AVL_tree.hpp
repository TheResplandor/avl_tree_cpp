/*
Purpose:    AVL_tree class declaration.
*/

#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#ifdef DEBUG
#include <expected>
#include <string>
#endif // ifdef DEBUG

enum class avl_statuses {
    UNINITIALIZED = -1,
    SUCCESS = 0,
    VALUE_NOT_FOUND,
};

static int8_t const SMALLER_UB = -2;
static int8_t const SMALLER_HEAVY = -1;
static int8_t const SMALLER_SIGN = -1;
static int8_t const BALANCED = 0;
static int8_t const BIGGER_SIGN = 1;
static int8_t const BIGGER_HEAVY = 1;
static int8_t const BIGGER_UB = 2;

template <std::totally_ordered T>
class AVL_tree {
public:
    AVL_tree() = default;
    explicit AVL_tree(
        T head_value): // TODO is this best to get the value as && and force the
                       // user to use std::move?
                       // on the one hand it makes it clear that "you lose whatever value is
                       // stored in that variable". on the other hand, it might prevent
                       // optimizations like RVO. plus, does it make usage less comfortable?
                       // It doesnt compile with primitive values and && so idk what to do.
        m_head(std::make_unique<AVL_node>(head_value))
    {
    }

    void add(T value)
    {
        if (m_head == nullptr) {
            m_head = std::make_unique<AVL_node>(value);
            return;
        }

        AVL_node* parent = nullptr;
        auto searched_node = m_head->find(value, &parent);
        // value already exists.
        if (searched_node != nullptr) {
            searched_node->m_count += 1;
            return;
        }

        auto new_node = std::make_unique<AVL_node>(value, parent);
        bool is_smaller_child = false;
        if (value < parent->m_value) {
            parent->m_smaller = std::move(new_node);
            is_smaller_child = true;
        } else {
            parent->m_bigger = std::move(new_node);
            is_smaller_child = false;
        }
        parent->rebalance_uptree(is_smaller_child, 1);
        return;
    }

    avl_statuses remove(T const value)
    {
        AVL_node* to_remove = nullptr;
        std::unique_ptr<AVL_node> replacement = nullptr;

        to_remove = m_head->find(value, nullptr);
        if (to_remove == nullptr) {
            return avl_statuses::VALUE_NOT_FOUND;
        }

        if (to_remove->m_count > 1) {
            to_remove->m_count -= 1;
            return avl_statuses::SUCCESS;
        }

        /* to_remove has 2 children - swap its value with its minimal bigger
        child's value and remove the child. */
        if ((to_remove->m_bigger != nullptr) && (to_remove->m_smaller != nullptr)) {
            AVL_node* to_swap = nullptr;

            to_swap = to_remove->m_bigger->get_min();
            to_remove->m_value = std::move(to_swap->m_value);
            to_remove->m_count = to_swap->m_count;
            // no need to re-assign to_swap's values since we are about to remove it.

            /* to_swap, being the minimum of its tree, has no left child,
            so just remove it with a case of 1 or 0 children. */
            to_remove = to_swap;
            to_swap = nullptr;
        }

        // Get to_remove's child if exists, or nullptr if it has no child.
        auto parent = to_remove->m_parent;
        if (to_remove->m_smaller != nullptr) {
            replacement = std::move(to_remove->m_smaller);
            replacement->m_parent = parent;
        } else if (to_remove->m_bigger != nullptr) {
            replacement = std::move(to_remove->m_bigger);
            replacement->m_parent = parent;
        }

        bool is_removing_smaller = false;

        // Only the tree's root has no parent.
        if (parent == nullptr) {
            m_head = std::move(replacement);
            return avl_statuses::SUCCESS;
        }
        if (to_remove == parent->m_smaller.get()) {
            parent->m_smaller = std::move(replacement);
            is_removing_smaller = true;
        } else {
            parent->m_bigger = std::move(replacement);
            is_removing_smaller = false;
        }

        parent->rebalance_uptree(is_removing_smaller, -1);

        return avl_statuses::SUCCESS;
    }

    bool contains(T const value) const
    {
        return m_head->find(value, nullptr) != nullptr;
    }

#ifdef DEBUG
    /**
     * @brief Prints the tree to the console.
     *
     * @note    * Debug function.
     *          * Only looks good for values whose char representation is the
     *              size of 1.
     */
    void print_tree()
    {
        size_t height = AVL_node::get_height(m_head.get());

        for (size_t i = 0; i < height; ++i) {
            m_head->print_nth_depth(i, height);
            std::cout << "\n";
        }
    }

    std::string test_tree()
    {
        if (m_head == nullptr) {
            return "";
        }
        auto test_out = m_head->test_subtree();
        if (!test_out) {
            return test_out.error();
        }
        return "";
    }
#endif // ifdef DEBUG

private:
    enum class node_statuses {
        UNINITIALIZED = -1,
        SUCCESS = 0,
        INVALID_HEIGHT,
    };

    class AVL_node {
    public:
        AVL_node(T& value, AVL_node* parent):
            m_smaller(nullptr),
            m_bigger(nullptr),
            m_parent(parent),
            m_value(std::move(value)),
            m_count(1),
            m_balance(BALANCED)
        {
        }

        AVL_node(T& value):
            AVL_node(value, nullptr)
        {
        }

        /**
         * @brief simple cleanup which relies on unique_ptr's recursive release properties.
         *
         * @note With 10^15 nodes, the worst AVL tree height is 72, so recursion is safe.
         */
        ~AVL_node()
        {
            m_smaller.reset();
            m_bigger.reset();
        }

        /**
         * @brief Search for a value in the subtree and return its node if
         * found, and optionaly its (potential) parent.
         *
         * @param[in] value The node value to search for
         * @param[out] parent Optional parent of the node or potential parent if not found.
         *
         * @returns The node if found, nullptr otherwise.
         */
        AVL_node* find(T const& value, AVL_node** parent)
        {
            AVL_node* curr_node = this;
            AVL_node* curr_parent = nullptr;

            while ((curr_node != nullptr) && (value != curr_node->m_value)) {
                curr_parent = curr_node;
                if (value < curr_node->m_value) {
                    curr_node = curr_node->m_smaller.get();
                } else {
                    curr_node = curr_node->m_bigger.get();
                }
            }

            if (parent != nullptr) {
                *parent = curr_parent;
            }
            return curr_node;
        }

        /**
         * @brief Get the minimal node in the subtree.
         */
        AVL_node* get_min()
        {
            AVL_node* curr_node = this;
            while (curr_node->m_smaller != nullptr) {
                curr_node = curr_node->m_smaller.get();
            }

            return curr_node;
        }

        bool operator<(const AVL_node& other) const
        {
            return m_value < other.m_value;
        }

        static size_t get_height(AVL_node const* node)
        {
            if (node == nullptr) {
                return 0;
            }

            return 1 +
                std::max(get_height(node->m_smaller.get()), get_height(node->m_bigger.get()));
        }

        void rebalance_uptree(bool smaller_called, int8_t balance_change)
        {
            bool keep_rebalancing = ((m_balance == BALANCED) == (balance_change == 1));

            if (smaller_called) {
                m_balance += SMALLER_SIGN * balance_change;
            } else {
                // bigger child called.
                m_balance += BIGGER_SIGN * balance_change;
            }
            auto parent = m_parent;

            if (m_balance == SMALLER_UB) {
                this->rotate_to_smaller();
            } else if (m_balance == BIGGER_UB) {
                this->rotate_to_bigger();
            }

            if (balance_change == -1) {
                keep_rebalancing = keep_rebalancing && (m_balance == BALANCED);
            }

            if ((parent != nullptr) && keep_rebalancing) {
                parent->rebalance_uptree(this == parent->m_smaller.get(), balance_change);
            }
        }

#ifdef DEBUG
        node_statuses print_nth_depth(size_t depth, size_t height, bool is_empty = false)
        {
            const char FILLER_CHAR = ' ';
            const char BRANCH_CHAR = '_';
            node_statuses status = node_statuses::UNINITIALIZED;
            size_t power = 0;

            if (height == 0) {
                return node_statuses::INVALID_HEIGHT;
            }

            if (depth == 0) {
                if (is_empty) {
                    for (size_t i = 1; i < std::pow(2, height); ++i) {
                        std::cout << FILLER_CHAR;
                    }
                    return node_statuses::SUCCESS;
                }

                if (height == 1) {
                    // std::cout << int(m_balance) + 1;
                    std::cout << m_value;
                    return node_statuses::SUCCESS;
                }

                power = height - 2;
                for (size_t i = 0; i < std::pow(2, power); ++i) {
                    std::cout << FILLER_CHAR;
                }
                for (size_t i = 0; i < std::pow(2, power) - 1; ++i) {
                    if (m_smaller == nullptr) {
                        std::cout << FILLER_CHAR;
                    } else {
                        std::cout << BRANCH_CHAR;
                    }
                }

                // std::cout << int(m_balance) + 1;
                std::cout << m_value;

                for (size_t i = 0; i < std::pow(2, power) - 1; ++i) {
                    if (m_bigger == nullptr) {
                        std::cout << FILLER_CHAR;
                    } else {
                        std::cout << BRANCH_CHAR;
                    }
                }
                for (size_t i = 0; i < std::pow(2, power); ++i) {
                    std::cout << FILLER_CHAR;
                }
            } else {
                if (is_empty || (m_smaller == nullptr)) {
                    status = this->print_nth_depth(depth - 1, height - 1, true);
                    if (status != node_statuses::SUCCESS) {
                        return status;
                    }
                } else {
                    status = m_smaller->print_nth_depth(depth - 1, height - 1);
                    if (status != node_statuses::SUCCESS) {
                        return status;
                    }
                }
                std::cout << FILLER_CHAR;
                if (is_empty || (m_bigger == nullptr)) {
                    status = this->print_nth_depth(depth - 1, height - 1, true);
                    if (status != node_statuses::SUCCESS) {
                        return status;
                    }
                } else {
                    status = m_bigger->print_nth_depth(depth - 1, height - 1);
                    if (status != node_statuses::SUCCESS) {
                        return status;
                    }
                }
            }

            return node_statuses::SUCCESS;
        }

        /**
         * @brief
         *
         * @return height of subtree, message explaining why its not a valid tree. message is empty
         * if it is a valid tree. value of height is only valid if message is empty (no error).
         */
        std::expected<size_t, std::string> test_subtree()
        {
            size_t height_smaller = 0;
            size_t height_bigger = 0;

            if (!((m_balance <= BIGGER_HEAVY) && (m_balance >= SMALLER_HEAVY))) {
                return std::unexpected { "saved balance is: " + std::to_string(m_balance) };
            }
            if ((m_smaller == nullptr) && (m_bigger == nullptr)) {
                return 1;
            }

            if (m_bigger != nullptr) {
                if (m_value >= m_bigger->m_value) {
                    return std::unexpected { "value >= bigger value" };
                }

                auto bigger_out = m_bigger->test_subtree();
                if (!bigger_out) {
                    return std::unexpected { bigger_out.error() };
                }
                height_bigger = *bigger_out;
            }

            if (m_smaller != nullptr) {
                if (m_value <= m_smaller->m_value) {
                    return std::unexpected { "value <= smaller_value" };
                }

                auto smaller_out = m_smaller->test_subtree();
                if (!smaller_out) {
                    return std::unexpected { smaller_out.error() };
                }
                height_smaller = *smaller_out;
            }

            auto balance =
                std::max(height_bigger, height_smaller) - std::min(height_smaller, height_bigger);
            if (balance >= 2) {
                return std::unexpected { "calculated balance is: " + std::to_string(balance) };
            }

            return std::max(height_bigger, height_smaller) + 1;
        }
#endif // ifdef DEBUG

        std::unique_ptr<AVL_node> m_smaller = nullptr;
        std::unique_ptr<AVL_node> m_bigger = nullptr;
        AVL_node* m_parent = nullptr;
        T m_value;
        uint32_t m_count;
        int8_t m_balance;

    private:
        void rotate_to_smaller(bool is_first_rotation = true)
        {
            if ((m_smaller->m_balance == BIGGER_HEAVY) && (is_first_rotation)) {
                m_smaller->rotate_to_bigger(false);
            }
            // Swap contents of this and m_bigger.
            // The ownership of the head node is inaccessible so changing location is the only
            // solution.
            swap_contents(this, m_smaller.get());

            // Move subtrees to achieve BST property again.
            std::swap(m_bigger, m_smaller);
            std::swap(m_bigger->m_smaller, m_bigger->m_bigger);
            std::swap(m_bigger->m_bigger, m_smaller);
            if (m_smaller != nullptr) {
                m_smaller->m_parent = this;
            }
            if (m_bigger->m_smaller != nullptr) {
                m_bigger->m_smaller->m_parent = m_bigger.get();
            }
            if (m_bigger->m_bigger != nullptr) {
                m_bigger->m_bigger->m_parent = m_bigger.get();
            }

            auto tmp = m_balance;
            if ((tmp == SMALLER_HEAVY) && (m_bigger->m_balance == SMALLER_HEAVY)) {
                m_balance = BIGGER_HEAVY;
            } else if ((tmp == SMALLER_UB) && (m_bigger->m_balance == SMALLER_UB)) {
                m_balance = BALANCED;
            } else {
                m_balance = m_bigger->m_balance + BIGGER_HEAVY;
            }

            if ((tmp == SMALLER_HEAVY) && (m_bigger->m_balance == BIGGER_HEAVY)) {
                m_bigger->m_balance = BALANCED;
            } else {
                m_bigger->m_balance = tmp + BIGGER_HEAVY - m_bigger->m_balance;
            }
        }

        void rotate_to_bigger(bool is_first_rotation = true)
        {
            if ((m_bigger->m_balance == SMALLER_HEAVY) && (is_first_rotation)) {
                m_bigger->rotate_to_smaller(false);
            }
            // Swap contents of this and m_bigger.
            // The ownership of the head node is inaccessible so changing location is the only
            // solution.
            swap_contents(this, m_bigger.get());

            // Move the subtrees to achieve BST property again.
            std::swap(m_smaller, m_bigger);
            std::swap(m_smaller->m_smaller, m_smaller->m_bigger);
            std::swap(m_smaller->m_smaller, m_bigger);
            if (m_bigger != nullptr) {
                m_bigger->m_parent = this;
            }
            if (m_smaller->m_smaller != nullptr) {
                m_smaller->m_smaller->m_parent = m_smaller.get();
            }
            if (m_smaller->m_bigger != nullptr) {
                m_smaller->m_bigger->m_parent = m_smaller.get();
            }

            auto tmp = m_balance;
            if ((tmp == BIGGER_HEAVY) && (m_smaller->m_balance == BIGGER_HEAVY)) {
                m_balance = SMALLER_HEAVY;
            } else if ((tmp == BIGGER_UB) && (m_smaller->m_balance == BIGGER_UB)) {
                m_balance = BALANCED;
            } else {
                m_balance = m_smaller->m_balance + SMALLER_HEAVY;
            }

            if ((tmp == BIGGER_HEAVY) && (m_smaller->m_balance == SMALLER_HEAVY)) {
                m_smaller->m_balance = BALANCED;
            } else {
                m_smaller->m_balance = tmp + SMALLER_HEAVY - m_smaller->m_balance;
            }
        }

        static void swap_contents(AVL_node* first, AVL_node* second)
        {
            std::swap(first->m_value, second->m_value);
            std::swap(first->m_count, second->m_count);
        }
    };

    std::unique_ptr<AVL_node> m_head = nullptr;
};

#endif // AVL_TREE_H