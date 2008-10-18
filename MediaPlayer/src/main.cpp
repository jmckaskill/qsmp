#include <iostream>
#include "PlaybackControls.h"
#include "LuaTcpConsole.h"
#include "PlaylistModel.h"
#include "PlaylistModel.inl"

namespace 
{
  struct Entry
  {
    Entry(const directory_entry& dir)
      : path_(dir.path())
    {
      init();
    }
    Entry(const Path& path)
      : path_(path)
    {
      init();
    }
    void init()
    {
#if 1
      ID3_Tag tag(path_.file_string().c_str());
      ID3_Frame* frame = tag.Find(ID3FID_LEADARTIST);
      if (frame)
      {
        ID3_Field* field = frame->GetField(ID3FN_TEXT);
        if (field)
          artist_ = field->GetRawText();
      }
#endif
    }
    std::string artist_;
    Path path_;
  };

  template<class Pred>
  struct extension_pred_t
  {
    extension_pred_t(const Pred& pred)
      : pred_(pred)
    {}
    bool operator()(const Entry& entry)const
    {
      return pred_(entry.path_.extension());
    }

    template<class Path>
    bool operator()(const boost::filesystem::basic_directory_entry<Path>& dir)const
    {
      return pred_(dir.path().extension());
    }
    template<class String, class Traits>
    bool operator()(const boost::filesystem::basic_path<String,Traits>& path)const
    {
      return pred_(path.extension());
    }
    Pred     pred_;
  };

  template<class Pred>
  extension_pred_t<Pred> is_extension(const Pred& pred)
  {return extension_pred_t<Pred>(pred);}

  struct equals
  {
    typedef bool result_type;
    template<typename Range1T, typename Range2T, typename PredicateT>
    bool operator()(const Range1T& Input, 
                    const Range2T& Test,
                    PredicateT Comp)const
    {
      return boost::equals(Input,Test,Comp);
    }
  };

  template<class Pred, class OutputIterator, class ValueType = typename boost::remove_reference<typename OutputIterator::reference>::type>
  class output_filter_iterator
    : public std::iterator_traits<OutputIterator>
  {
  public:
    output_filter_iterator(const Pred& pred,OutputIterator output)
      : output_(output),pred_(pred){}
    output_filter_iterator&
      operator=(const ValueType& val)
    {
      if (pred_(val))
        *output_ = val;
      return *this;
    }

    output_filter_iterator& operator*()
    {return *this;}

    output_filter_iterator& operator++()
    {	
      ++output_;
      return *this;
    }

    output_filter_iterator operator++(int)
    {	
      OutputIterator ret(*this);
      ++output_;
      return ret;
    }

  protected:
    OutputIterator output_;
    Pred           pred_;
  };

  template<class Pred, class OutputIterator>
  output_filter_iterator<Pred,OutputIterator> make_output_filter_iterator(const Pred& pred, OutputIterator output)
  {
    return output_filter_iterator<Pred,OutputIterator>(pred,output);
  }

  template<class ValueType, class Pred, class OutputIterator>
  output_filter_iterator<Pred,OutputIterator,ValueType> make_ofi_with_value_type(const Pred& pred, OutputIterator output)
  {
    return output_filter_iterator<Pred,OutputIterator,ValueType>(pred,output);
  }
}

int main(int argc, char **argv)
{
  try
  {
    QApplication app(argc, argv);
    app.setApplicationName("SMPMediaPlayer");
    app.setApplicationVersion("0.0.1");
    app.setOrganizationName("Foobar NZ");
    app.setOrganizationDomain("foobar.co.nz");

    MainWin mywindow;
    //mywindow.show();


    std::vector<Entry> paths;
    std::copy(recursive_directory_iterator("E:/Music/Andrew"),
              recursive_directory_iterator(),
              make_ofi_with_value_type<recursive_directory_iterator::value_type>(
                  is_extension(bind(equals(),
                                    _1,
                                    string(".mp3"),
                                    boost::is_iequal())),
                  std::back_inserter(paths)));

    typedef PlaylistModel<std::vector<Entry>& > Model_t;

    Model_t* model = new Model_t(paths);
    QTableView* view = new QTableView;
    view->setModel(model);

    QMainWindow view_window;
    view_window.setCentralWidget(view);
    view_window.show();


    LuaTcpServer lua;

    return app.exec();
  }
  catch(std::exception& e)
  {
    qFatal("Exception: %s",e.what());
  }
}
