/******************************************************************************
 * Copyright (C) 2008 James McKaskill <jmckaskill@gmail.com>                  *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License as             *
 * published by the Free Software Foundation; either version 2 of             *
 * the License, or (at your option) any later version.                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.      *
 ******************************************************************************/

#ifndef MEDIAPLAYER_PLAYLIST
#define MEDIAPLAYER_PLAYLIST

#ifdef FOR_REFERENCE

template<class CharIter, class Index>
struct FilteredIndex
{
  CharIter begin_;
  CharIter end_;
  Index    index_;
  bool operator<(const FilteredIndex& r)const
  {return index_ < r.index_;}
};

enum ECacheType
{
};


template<class CharIter>
struct CacheEntry
{
  
};




template<class FilteredIndexIter, class Char, class CompPred>
FilteredIndexIter
refine_filter_end(FilteredIndexIter filteredIndexesBegin, FilteredIndexIter filteredIndexesEnd, 
                  Char refinement, CompPred compPred = std::equal())
{
  struct Partition
  {
    Partition(CompPred compPred, Char refinement):compPred_(compPred),refinement_(refinement){}
    bool operator()(std::iterator_traits<FilteredIndexIter>::reference index)
    {
      return compPred_(index.end,refinement_);
    }
    CompPred compPred_;
    Char refinement_;
  };
  FilteredIndexIter ret = std::stable_partition(filteredIndexesBegin,filteredIndexesEnd,
                                                Partition(compPred,refinement));
  for(FilteredIndexIter ii = filteredIndexesBegin;ii != ret; ++ii)
  {
    ++(ii->end);
  }
  return ret;
}

template<class FilteredIndexIter> inline
void unrefine_filter_end(FilteredIndexIter begin, FilteredIndexIter end, FilteredIndexIter oldEnd)
{
  std::inplace_merge(begin,end,oldEnd);
}


template<class Iter, class MiniIndex>
struct Index
{
  MiniIndex index_;
  Iter data_;
};

template<class Range, class APCPool>
void sortPlaylist(const Range& playlist, const Range& activeRange, const APCPool& apcPool)
{
  

}

#endif






#endif