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

const char FILLER_CHAR = ' ';
const char BRANCH_CHAR = '_';

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
    AVL_tree(T const& head_value):
        m_head(std::make_unique<AVL_node>(head_value, nullptr, nullptr))
    {
    }

    avl_statuses add(T const& value)
    {
        auto new_node = std::make_unique<AVL_node>(value);
        AVL_node* parent = nullptr;

        if (nullptr == this->m_head) {
            this->m_head = std::move(new_node);
            return avl_statuses::SUCCESS;
        }

        m_head->find_potential_parent(value, parent);

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

        this->m_head->find(value, to_remove, parent);
        if (nullptr == to_remove) {
            return avl_statuses::VALUE_NOT_FOUND;
        }

        /* to_remove has 2 children - swap its value with its minimal bigger
        child's value and remove the child. */
        if ((nullptr != to_remove->m_bigger)
            && (nullptr != to_remove->m_smaller)) {
            AVL_node* parent_of_min = nullptr;
            AVL_node* to_swap = nullptr;

            to_remove->m_bigger->get_parent_of_min(parent_of_min);
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
        replacement
            = std::move(nullptr == to_remove->m_smaller ? to_remove->m_bigger
                                                        : to_remove->m_smaller);

        // Only the tree's root has no parent.
        if (nullptr == parent) {
            this->m_head = std::move(replacement);
        } else {
            if (parent->m_smaller.get() == to_remove) {
                parent->m_smaller = std::move(replacement);
            } else {
                parent->m_bigger = std::move(replacement);
            }
        }

        return avl_statuses::SUCCESS;
    }

    bool is_inside(T const& value) const
    {
        AVL_node* parent;
        AVL_node* node;

        this->m_head->find(value, node, parent);

        return nullptr != node;
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
        size_t height = AVL_node::get_height(this->m_head.get());

        for (size_t i = 0; i < height; ++i) {
            this->m_head->print_nth_depth(i, height);
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
        AVL_node(T const& value, std::unique_ptr<AVL_node>&& left,
            std::unique_ptr<AVL_node>&& right):
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
         * @brief Search for a value in the subtree and return its node if
         * found, and its (potential) parent.
         *
         * @param[in] value The node value to search for
         * @param[out] node The node if found, nullptr otherwise.
         * @param[out] parent Parent of the node or potential parent if not
         * found.
         */
        void find(T const& value, AVL_node*& node, AVL_node*& parent)
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

            node = curr_node;
            parent = curr_parent;
        }

        void find_potential_parent(T const& value, AVL_node*& parent)
        {
            AVL_node* curr_node = nullptr;
            AVL_node* curr_parent = nullptr;

            do {
                find(value, curr_node, curr_parent);
                if (curr_node != nullptr) {
                    curr_parent = curr_node;
                    curr_node = curr_node->m_smaller.get();
                }
            } while (curr_node != nullptr);

            parent = curr_parent;
        }

        /**
         * @brief Get the parent of the minimum node in the subtree.
         *
         * @param[out] parent The parent of the node, or nullptr if the
         * minimum is the calling node.
         */
        void get_parent_of_min(AVL_node*& parent)
        {
            AVL_node* curr_node = this;
            AVL_node* curr_parent = nullptr;
            while (nullptr != curr_node->m_smaller) {
                curr_parent = curr_node;
                curr_node = curr_node->m_smaller.get();
            }

            parent = curr_parent;
        }

        bool operator<(const AVL_node& other) const
        {
            return *(this->m_value) < *(other.m_value);
        }

        static size_t get_height(AVL_node const* node)
        {
            if (nullptr == node) {
                return 0;
            }

            return 1
                + std::max(get_height(node->m_smaller.get()),
                    get_height(node->m_bigger.get()));
        }

        node_statuses print_nth_depth(
            size_t depth, size_t height, bool is_empty = false)
        {
            node_statuses status = node_statuses::UNINITIALIZED;
            size_t power = 0;

            if (0 == height) {
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
                    std::cout << *(this->m_value);
                    return node_statuses::SUCCESS;
                }

                power = height - 2;
                for (size_t i = 0; i < std::pow(2, power); ++i) {
                    std::cout << FILLER_CHAR;
                }
                for (size_t i = 0; i < std::pow(2, power) - 1; ++i) {
                    if (nullptr == this->m_smaller) {
                        std::cout << FILLER_CHAR;
                    } else {
                        std::cout << BRANCH_CHAR;
                    }
                }

                std::cout << *(this->m_value);

                for (size_t i = 0; i < std::pow(2, power) - 1; ++i) {
                    if (nullptr == this->m_bigger) {
                        std::cout << FILLER_CHAR;
                    } else {
                        std::cout << BRANCH_CHAR;
                    }
                }
                for (size_t i = 0; i < std::pow(2, power); ++i) {
                    std::cout << FILLER_CHAR;
                }
            } else {
                if (is_empty || nullptr == this->m_smaller) {
                    status = this->print_nth_depth(depth - 1, height - 1, true);
                    if (node_statuses::SUCCESS != status) {
                        return status;
                    }
                } else {
                    status = this->m_smaller->print_nth_depth(
                        depth - 1, height - 1);
                    if (node_statuses::SUCCESS != status) {
                        return status;
                    }
                }
                std::cout << FILLER_CHAR;
                if (is_empty || nullptr == this->m_bigger) {
                    status = this->print_nth_depth(depth - 1, height - 1, true);
                    if (node_statuses::SUCCESS != status) {
                        return status;
                    }
                } else {
                    status = this->m_bigger->print_nth_depth(
                        depth - 1, height - 1);
                    if (node_statuses::SUCCESS != status) {
                        return status;
                    }
                }
            }

            return node_statuses::SUCCESS;
        }

        std::unique_ptr<AVL_node> m_smaller = nullptr;
        std::unique_ptr<AVL_node> m_bigger = nullptr;
        std::unique_ptr<T> m_value;
    };

    std::unique_ptr<AVL_node> m_head = nullptr;
};

#endif // AVL_TREE_H