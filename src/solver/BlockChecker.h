/* (c) 2020 RNDr. Simon Toth (happy.cerberus@gmail.com) */
#ifndef SUDOKU_BLOCKCHECKER_H
#define SUDOKU_BLOCKCHECKER_H

#include "Square.h"
#include <cstdint>
#include <unordered_set>
#include <vector>

namespace sudoku {
    class BlockChecker {
    public:
        // Removed default constructor.
        BlockChecker() = delete;

        /*! Construct block checker on top of a set of squares.
         *
         * @param elements Pointers to squares.
         */
        BlockChecker(const std::vector<Square *> &elements);

        Square *GetSquare(unsigned pos) const {
            return elem_[pos];
        }

        /*! Check whether block has a conflict (number appears multiple times)
         *  Works for partially filled blocks.
         *
         * @return True if no conflict found, False otherwise.
         */
        bool HasConflict() const;

        /*! Remove a number as a possibility from a block.
         *
         * @param number Number to be removed.
         */
        void Prune(unsigned number);

        /*! Remove a number as a possibility from a block.
         *
         * @param number Number to be removed.
         * @param whitelist Blocks to skip over.
         */
        void Prune(unsigned number, const std::vector<unsigned> &whitelist);

        /*! Remove a number as a possibility from a block.
         *
         * @param number Number to be removed.
         * @param whitelist Blocks to skip over.
         */
        void Prune(unsigned number, std::vector<unsigned>::const_iterator begin,
                   std::vector<unsigned>::const_iterator end);

        /*! Return possible positions for a number.
         *
         * @param number Number to search for.
         * @return set of positions.
         */
        std::unordered_set<unsigned> NumberPositions(unsigned number) const;

        /*! Insert possible positions for a number into the given set.
         *
         * @param number Number to search for.
         * @param positions Set to insert into.
         */
        void NumberPositions(unsigned number,
                             std::unordered_set<unsigned> &positions) const;

        /*! Determine whether this block has the number at a given position.
         *
         * @param number Number to search for.
         * @param position Position to check.
         */
        bool HasNumberAtPosition(unsigned number, unsigned position) const;

        /*! Return the size of the block.
         *
         * @return Size of the block.
         */
        unsigned Size() const { return static_cast<unsigned>(elem_.size()); }

        /*! Find numbers that are fully contained within an intersection with another
         *  block, and remove those from non-intersection squares of the other block.
         *
         *  @param r Other block to intersect with and prune.
         */
        void SolveIntersection(BlockChecker &r) const;

        //! Solve the block for hidden groups.
        void SolveHiddenGroups() const;

        /*! Solve the block for hidden groups of a given size.
         *
         * @param size Size of the group to look for.
         */
        void SolveHiddenGroups(unsigned size) const;

        //! Solve the block for naked groups.
        void SolveNakedGroups() const;

        /*! Solve the block for naked groups of a given size.
         *
         * @param size Size of the group to look for.
         */
        void SolveNakedGroups(unsigned int size) const;


    private:
        std::vector<Square *> elem_;

        friend std::vector<Square *> &TestGetBlockData(BlockChecker &s);
    };

    void recursive_set_find(std::vector<unsigned> &result,
                            const std::vector<sudoku::Square *> &squares,
                            unsigned size);

    void recursive_number_find(std::vector<unsigned> &result,
                               const std::vector<sudoku::Square *> &squares,
                               unsigned size);

// TODO: write tests
    void recursive_fish_find(std::vector<unsigned> &result,
                             const std::vector<BlockChecker *> &blocks,
                             unsigned size, unsigned number);

    void recursive_finned_fish_find(std::vector<unsigned> &result,
                                    const std::vector<BlockChecker *> &blocks,
                                    unsigned size, unsigned number);

} // namespace sudoku

#endif // SUDOKU_BLOCKCHECKER_H
