#ifndef MEDIAPLAYER_PLAYLIST
#define MEDIAPLAYER_PLAYLIST


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