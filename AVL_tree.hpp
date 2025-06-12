/*
Purpose:    AVL_tree class declaration.
*/

#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

enum class avl_statuses {
    UNINITIALIZED = -1,
    SUCCESS = 0,
    VALUE_NOT_FOUND,
};

enum class balance_state : int8_t {
    SMALLER_UB = -2,
    SMALLER_HEAVY = -1,
    BALANCED = 0,
    BIGGER_HEAVY = 1,
    BIGGER_UB = 2,
};

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
        if (value < parent->m_value) {
            parent->m_smaller = std::move(new_node);
        } else {
            parent->m_bigger = std::move(new_node);
        }
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
        if (to_remove->m_smaller != nullptr) {
            replacement = std::move(to_remove->m_smaller);
            replacement->m_parent = to_remove->m_parent;
        } else if (to_remove->m_bigger != nullptr) {
            replacement = std::move(to_remove->m_bigger);
            replacement->m_parent = to_remove->m_parent;
        }

        // Only the tree's root has no parent.
        if (to_remove->m_parent == nullptr) {
            m_head = std::move(replacement);
        } else {
            if (to_remove == to_remove->m_parent->m_smaller.get()) {
                to_remove->m_parent->m_smaller = std::move(replacement);
            } else {
                to_remove->m_parent->m_bigger = std::move(replacement);
            }
        }

        return avl_statuses::SUCCESS;
    }

    bool contains(T const value) const
    {
        return m_head->find(value, nullptr) != nullptr;
    }

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
            m_balance(balance_state::BALANCED)
        {
        }

        AVL_node(T& value):
            AVL_node(value, nullptr)
        {
        }

        /**
         * @brief safe cleanup of the subtree.
         *
         * This avoids recursion or dynamic alloactions, while working in O(n).
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

        std::unique_ptr<AVL_node> m_smaller = nullptr;
        std::unique_ptr<AVL_node> m_bigger = nullptr;
        AVL_node* m_parent = nullptr;
        T m_value;
        uint32_t m_count;
        balance_state m_balance;
    };

    std::unique_ptr<AVL_node> m_head = nullptr;
};

#endif // AVL_TREE_H