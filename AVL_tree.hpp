/*
Purpose:    AVL_tree class declaration.
*/

#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <algorithm>
#include <cmath>
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
    VALUE_EXISTS,
};

template <typename T>
class AVL_tree {
public:
    AVL_tree() = default;
    explicit AVL_tree(T const& head_value):
        m_head(std::make_unique<AVL_node>(head_value, nullptr, nullptr))
    {
    }

    avl_statuses add(T const& value)
    {
        auto new_node = std::make_unique<AVL_node>(value);
        AVL_node* parent = nullptr;

        if (m_head == nullptr) {
            m_head = std::move(new_node);
            return avl_statuses::SUCCESS;
        }

        parent = m_head->find_potential_parent(value);

        if (*(new_node->m_value) <= *(parent->m_value)) {
            parent->m_smaller = std::move(new_node);
        } else {
            parent->m_bigger = std::move(new_node);
        }
        return avl_statuses::SUCCESS;
    }

    avl_statuses remove(T const& value)
    {
        AVL_node* to_remove = nullptr;
        AVL_node* parent = nullptr;
        std::unique_ptr<AVL_node> replacement = nullptr;

        to_remove = m_head->find(value, &parent);
        if (to_remove == nullptr) {
            return avl_statuses::VALUE_NOT_FOUND;
        }

        /* to_remove has 2 children - swap its value with its minimal bigger
        child's value and remove the child. */
        if ((to_remove->m_bigger != nullptr) && (to_remove->m_smaller != nullptr)) {
            AVL_node* parent_of_min = nullptr;
            AVL_node* to_swap = nullptr;

            parent_of_min = to_remove->m_bigger->get_parent_of_min();
            if (parent_of_min == nullptr) {
                to_swap = to_remove->m_bigger.get();
                parent_of_min = to_remove;
            } else {
                to_swap = parent_of_min->m_smaller.get();
            }

            to_remove->m_value = std::move(to_swap->m_value);

            /* to_swap, being the minimum of its tree, has no left child,
            so just remove it with a case of 1 or 0 children. */
            to_remove = to_swap;
            parent = parent_of_min;
            to_swap = nullptr;
            parent_of_min = nullptr;
        }

        // Get to_remove's child if exists, or nullptr if it has no child.
        replacement =
            std::move(to_remove->m_smaller == nullptr ? to_remove->m_bigger : to_remove->m_smaller);

        // Only the tree's root has no parent.
        if (parent == nullptr) {
            m_head = std::move(replacement);
        } else {
            if (to_remove == parent->m_smaller.get()) {
                parent->m_smaller = std::move(replacement);
            } else {
                parent->m_bigger = std::move(replacement);
            }
        }

        return avl_statuses::SUCCESS;
    }

    bool contains(T const& value) const
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
        AVL_node(
            T const& value, std::unique_ptr<AVL_node>&& left, std::unique_ptr<AVL_node>&& right):
            m_smaller(std::move(left)),
            m_bigger(std::move(right)),
            m_value(std::make_unique<T>(value))
        {
        }

        AVL_node(T const& value):
            AVL_node(value, nullptr, nullptr)
        {
        }

        /**
         * @brief safe cleanup of the subtree.
         *
         * This avoids recursion or dynamic alloactions, while working in O(n).
         */
        ~AVL_node()
        {
            destroy_tree(std::move(m_smaller));
            destroy_tree(std::move(m_bigger));
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

            while ((curr_node != nullptr) && (value != *(curr_node->m_value))) {
                curr_parent = curr_node;
                if (value < *(curr_node->m_value)) {
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

        AVL_node* find_potential_parent(T const& value)
        {
            AVL_node* curr_node = nullptr;
            AVL_node* curr_parent = nullptr;

            do {
                curr_node = find(value, &curr_parent);
                if (curr_node != nullptr) {
                    curr_parent = curr_node;
                    curr_node = curr_node->m_smaller.get();
                }
            } while (curr_node != nullptr);

            return curr_parent;
        }

        /**
         * @brief Get the parent of the minimum node in the subtree.
         *
         * @param[out] parent The parent of the node, or nullptr if the
         * minimum is the calling node.
         */
        AVL_node* get_parent_of_min()
        {
            AVL_node* curr_node = this;
            AVL_node* curr_parent = nullptr;
            while (curr_node->m_smaller != nullptr) {
                curr_parent = curr_node;
                curr_node = curr_node->m_smaller.get();
            }

            return curr_parent;
        }

        bool operator<(const AVL_node& other) const
        {
            return *(m_value) < *(other.m_value);
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
                    std::cout << *(m_value);
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

                std::cout << *(m_value);

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
        std::unique_ptr<T> m_value;

    private:
        /**
         * @brief Destroy the subtree starting at head.
         *
         * Does so without hidden recursion or dynamic allocations by reversing the direction of the
         * tree and only starts destroying nodes from the ends of the tree when they have no more
         * child nodes.
         *
         * @param head The head of the nodes tree to be destroyed.
         */
        void destroy_tree(std::unique_ptr<AVL_node> head)
        {
            std::unique_ptr<AVL_node> prev = nullptr;
            std::unique_ptr<AVL_node> next = nullptr;
            std::unique_ptr<AVL_node> curr = std::move(head);

            while (curr != nullptr) {
                if (curr->m_smaller != nullptr) {
                    // proceed to the smaller child node.
                    next = std::move(curr->m_smaller);
                    // make m_smaller point to the parent node.
                    curr->m_smaller = std::move(prev);
                    prev = std::move(curr);
                    curr = std::move(next);
                } else if (curr->m_bigger != nullptr) {
                    // proceed to the bigger child node.
                    next = std::move(curr->m_bigger);
                    // make m_smaller point to the parent node.
                    curr->m_smaller = std::move(prev);
                    prev = std::move(curr);
                    curr = std::move(next);
                } else {
                    // No child nodes so move "up" and current node is destroyed.
                    curr = std::move(prev);
                    if (curr != nullptr) {
                        // We are going up so m_smaller already points to the parent node.
                        prev = std::move(curr->m_smaller);
                    }
                }
            }
        }
    };

    std::unique_ptr<AVL_node> m_head = nullptr;
};

#endif // AVL_TREE_H