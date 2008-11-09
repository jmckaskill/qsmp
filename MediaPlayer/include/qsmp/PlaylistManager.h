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

#ifndef MEDIAPLAYER_PLAYLIST_MANAGER
#define MEDIAPLAYER_PLAYLIST_MANAGER


#ifdef FOR_REFERENCE

class CollectionVisitor;

typedef boost::iterator_range<char> Data;
typedef std::vector<std::pair<MetadataType, bool> > SortCriteria;


template<class Range1, class Range2>
int lexicographical_compare_3way(Range1 range1, Range2 range2)
{
  return strncmp(begin(range1),begin(range2),min(boost::size(range1),boost::size(range2)));
}

template<class CriteriaSequence>
struct MediaFileSort
{
  template<class CriteriaRange>
  MediaFileSort(CriteriaRange range)
  {
    std::copy(begin(range),end(range),begin(criteria_));
  }
  bool operator()(const MediaFile& l, const MediaFile& r)
  {
    CriteriaSequence::iterator ii = begin(criteria_);
    int lexo = 0;
    Data ldata,rdata;
    for(;ii != end(criteria_); ++ii)
    {
      ldata = l.metadata(*ii);
      rdata = r.metadata(*ii);
      lexo = lexicographical_compare_3way(ldata,rdata);
      if (lexo != 0)
        return lexo < 0;
    }
    return lexo < 0;
  }
  CriteriaSequence criteria_;
};

class Metadata
{
public:
  Metadata()
    : type_specific_(0),
      data_(NULL,NULL)
  {}
  int             type_specific()const{return type_specific_;}
  int             set_type_specific(int type_specific){type_specific_ = type_specific;}
  Data            data()const{return data_;}
  void            set_data(Data data){data_ = data;}
private:
  int             type_specific_;
  Data            data_;
};

class MediaFile
{
public:
  MediaFile()
  {}
  const Metadata& metadata(MetadataType type)const{return metadata_[type];}
  void            set_metadata(const Metadata& metadata){metadata_[type] = metadata;}
private:
  array<Metadata,METADATA_NUM> metadata_;
};


class ListCollection 
{
public:
  signal<void (MediaFile*,size_t)>             signalRemove;
  signal<void (MediaFile*,MediaFile*)>         signalBlockRemove;
  signal<void (MediaFile*,size_t)>             signalAdd;
  signal<void (MediaFile*,MediaFile*)>         signalBlockAdd;
  signal<void (MediaFile*,size_t,size_t)>      signalMove;
  signal<void (SortCriteria)>                  signalSort;
};

class ListCollectionBase : public ListCollection
{
protected:
  typedef std::vector<File*>::iterator iterator;
};

class DiskCollection
{
public:
  template<class Iter>
  DiskCollection(Iter begin, Iter end);
private:
  std::vector<> data_;
};

class DynamicCollection
{
public:
  template<class CollectionIter>
  DynamicCollection(CollectionIter begin, CollectionIter end);
private:
  std::vector<std::vector<CollectionData> > data_;
};

typedef boost::shared_ptr<Collection>            CollectionRef;
typedef std::vector<CollectionRef>::iterator     CollectionIter;
typedef boost::iterator_range<CollectionIter>    CollectionRange;

class CollectionManagerInterface
{
public:
  virtual CollectionRange getCollectionList()const=0;
  virtual CollectionRef   getCurrentCollection()const=0;
  virtual void            setCurrentCollection(CollectionRef collection)=0;
  virtual void            addCollection(CollectionRef collection)=0;
  virtual void            removeCollection(CollectionRef collection)=0;
private:

};
#endif

#endif