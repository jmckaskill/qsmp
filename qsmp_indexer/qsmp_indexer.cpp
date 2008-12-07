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

#include <boost/array.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/static_assert.hpp>
#include <id3/tag.h>
#include <iterator>
#include <iostream>
#include <string>
#include <string.h>
#ifdef WIN32
#include <fcntl.h> //for _O_BINARY
#include <io.h>    //for _setmode
#endif

namespace po = boost::program_options;
namespace fs = boost::filesystem;
using boost::scoped_ptr;

#define QSMPINDEXER_BEGIN namespace qsmp_indexer {
#define QSMPINDEXER_END  }


QSMPINDEXER_BEGIN

const char* id_lookup[] = {
  "no_field",//ID3FN_NOFIELD
  "text_encoding",//ID3FN_TEXTENC
  "text",//ID3FN_TEXT
  "url",//ID3FN_URL
  "data",//ID3FN_DATA
  "description",//ID3FN_DESCRIPTION
  "owner",//ID3FN_OWNER
  "email",//ID3FN_EMAIL
  "rating",//ID3FN_RATING
  "filename",//ID3FN_FILENAME
  "language",//ID3FN_LANGUAGE
  "picture_type",//ID3FN_PICTURETYPE
  "image_format",//ID3FN_IMAGEFORMAT
  "mime_type",//ID3FN_MIMETYPE
  "counter",//ID3FN_COUNTER
  "id",//ID3FN_ID
  "volume_adjustment",//ID3FN_VOLUMEADJ
  "number_bits",//ID3FN_NUMBITS
  "volume_change_right",//ID3FN_VOLCHGRIGHT
  "volume_change_left",//ID3FN_VOLCHGLEFT
  "peak_volume_right",//ID3FN_PEAKVOLRIGHT
  "peak_volume_left",//ID3FN_PEAKVOLLEFT
  "timestamp_format",//ID3FN_TIMESTAMPFORMAT
  "content_type"//ID3FN_CONTENTTYPE
};
BOOST_STATIC_ASSERT(sizeof(id_lookup)/sizeof(const char*) == ID3FN_LASTFIELDID);

const char* frame_lookup[] = {
  "unknown",              /* ???? ID3FID_NOFRAME = 0,       *< No known frame */
  "audio_crypto",         /* AENC ID3FID_AUDIOCRYPTO,       *< Audio encryption */
  "picture",              /* APIC ID3FID_PICTURE,           *< Attached picture */
  "audio_seekpoint",      /* ASPI ID3FID_AUDIOSEEKPOINT,    *< Audio seek point index */
  "comment",              /* COMM ID3FID_COMMENT,           *< Comments */
  "commercial",           /* COMR ID3FID_COMMERCIAL,        *< Commercial frame */
  "crypto_registration",  /* ENCR ID3FID_CRYPTOREG,         *< Encryption method registration */
  "equalization_2",       /* EQU2 ID3FID_EQUALIZATION2,     *< Equalisation (2) */
  "equalization",         /* EQUA ID3FID_EQUALIZATION,      *< Equalization */
  "event_timing",         /* ETCO ID3FID_EVENTTIMING,       *< Event timing codes */
  "general",              /* GEOB ID3FID_GENERALOBJECT,     *< General encapsulated object */
  "group_id",             /* GRID ID3FID_GROUPINGREG,       *< Group identification registration */
  "involved_people",      /* IPLS ID3FID_INVOLVEDPEOPLE,    *< Involved people list */
  "linked_info",          /* LINK ID3FID_LINKEDINFO,        *< Linked information */
  "cdid",                 /* MCDI ID3FID_CDID,              *< Music CD identifier */
  "mpeg_lookup",          /* MLLT ID3FID_MPEGLOOKUP,        *< MPEG location lookup table */
  "ownership",            /* OWNE ID3FID_OWNERSHIP,         *< Ownership frame */
  "private",              /* PRIV ID3FID_PRIVATE,           *< Private frame */
  "play_counter",         /* PCNT ID3FID_PLAYCOUNTER,       *< Play counter */
  "popularimeter",        /* POPM ID3FID_POPULARIMETER,     *< Popularimeter */
  "position_sync",        /* POSS ID3FID_POSITIONSYNC,      *< Position synchronisation frame */
  "buffer_size",          /* RBUF ID3FID_BUFFERSIZE,        *< Recommended buffer size */
  "volume_adjustment_2",  /* RVA2 ID3FID_VOLUMEADJ2,        *< Relative volume adjustment (2) */
  "volume_adjustment",    /* RVAD ID3FID_VOLUMEADJ,         *< Relative volume adjustment */
  "reverb",               /* RVRB ID3FID_REVERB,            *< Reverb */
  "seek",                 /* SEEK ID3FID_SEEKFRAME,         *< Seek frame */
  "signature",            /* SIGN ID3FID_SIGNATURE,         *< Signature frame */
  "sync_lyrics",          /* SYLT ID3FID_SYNCEDLYRICS,      *< Synchronized lyric/text */
  "sync_tempo",           /* SYTC ID3FID_SYNCEDTEMPO,       *< Synchronized tempo codes */
  "album",                /* TALB ID3FID_ALBUM,             *< Album/Movie/Show title */
  "bpm",                  /* TBPM ID3FID_BPM,               *< BPM (beats per minute) */
  "composer",             /* TCOM ID3FID_COMPOSER,          *< Composer */
  "content_type",         /* TCON ID3FID_CONTENTTYPE,       *< Content type */
  "copyright",            /* TCOP ID3FID_COPYRIGHT,         *< Copyright message */
  "date",                 /* TDAT ID3FID_DATE,              *< Date */
  "encoding_time",        /* TDEN ID3FID_ENCODINGTIME,      *< Encoding time */
  "playlist_delay",       /* TDLY ID3FID_PLAYLISTDELAY,     *< Playlist delay */
  "original_release_time",/* TDOR ID3FID_ORIGRELEASETIME,   *< Original release time */
  "recording_time",       /* TDRC ID3FID_RECORDINGTIME,     *< Recording time */
  "release_time",         /* TDRL ID3FID_RELEASETIME,       *< Release time */
  "tagging_time",         /* TDTG ID3FID_TAGGINGTIME,       *< Tagging time */
  "involved_people_2",    /* TIPL ID3FID_INVOLVEDPEOPLE2,   *< Involved people list */
  "encoded_by",           /* TENC ID3FID_ENCODEDBY,         *< Encoded by */
  "lyricist",             /* TEXT ID3FID_LYRICIST,          *< Lyricist/Text writer */
  "file_type",            /* TFLT ID3FID_FILETYPE,          *< File type */
  "time",                 /* TIME ID3FID_TIME,              *< Time */
  "content_group",        /* TIT1 ID3FID_CONTENTGROUP,      *< Content group description */
  "title",                /* TIT2 ID3FID_TITLE,             *< Title/songname/content description */
  "subtitle",             /* TIT3 ID3FID_SUBTITLE,          *< Subtitle/Description refinement */
  "initial_key",          /* TKEY ID3FID_INITIALKEY,        *< Initial key */
  "language",             /* TLAN ID3FID_LANGUAGE,          *< Language(s) */
  "length",               /* TLEN ID3FID_SONGLEN,           *< Length */
  "musician_credits",     /* TMCL ID3FID_MUSICIANCREDITLIST,*< Musician credits list */
  "media_type",           /* TMED ID3FID_MEDIATYPE,         *< Media type */
  "mood",                 /* TMOO ID3FID_MOOD,              *< Mood */
  "original_album",       /* TOAL ID3FID_ORIGALBUM,         *< Original album/movie/show title */
  "original_filename",    /* TOFN ID3FID_ORIGFILENAME,      *< Original filename */
  "original_lyricist",    /* TOLY ID3FID_ORIGLYRICIST,      *< Original lyricist(s)/text writer(s) */
  "original_artist",      /* TOPE ID3FID_ORIGARTIST,        *< Original artist(s)/performer(s) */
  "original_year",        /* TORY ID3FID_ORIGYEAR,          *< Original release year */
  "file_owner",           /* TOWN ID3FID_FILEOWNER,         *< File owner/licensee */
  "lead_artist",          /* TPE1 ID3FID_LEADARTIST,        *< Lead performer(s)/Soloist(s) */
  "band",                 /* TPE2 ID3FID_BAND,              *< Band/orchestra/accompaniment */
  "conductor",            /* TPE3 ID3FID_CONDUCTOR,         *< Conductor/performer refinement */
  "mix_artist",           /* TPE4 ID3FID_MIXARTIST,         *< Interpreted, remixed, or otherwise modified by */
  "part_in_set",          /* TPOS ID3FID_PARTINSET,         *< Part of a set */
  "produced_notice",      /* TPRO ID3FID_PRODUCEDNOTICE,    *< Produced notice */
  "publisher",            /* TPUB ID3FID_PUBLISHER,         *< Publisher */
  "track_number",         /* TRCK ID3FID_TRACKNUM,          *< Track number/Position in set */
  "recording_dates",      /* TRDA ID3FID_RECORDINGDATES,    *< Recording dates */
  "net_radio_station",    /* TRSN ID3FID_NETRADIOSTATION,   *< Internet radio station name */
  "net_radio_owner",      /* TRSO ID3FID_NETRADIOOWNER,     *< Internet radio station owner */
  "size",                 /* TSIZ ID3FID_SIZE,              *< Size */
  "album_sort_order",     /* TSOA ID3FID_ALBUMSORTORDER,    *< Album sort order */
  "performer_sort_order", /* TSOP ID3FID_PERFORMERSORTORDER,*< Performer sort order */
  "title_sort_order",     /* TSOT ID3FID_TITLESORTORDER,    *< Title sort order */
  "isrc",                 /* TSRC ID3FID_ISRC,              *< ISRC (international standard recording code) */
  "encoder_settings",     /* TSSE ID3FID_ENCODERSETTINGS,   *< Software/Hardware and settings used for encoding */
  "set_subtitle",         /* TSST ID3FID_SETSUBTITLE,       *< Set subtitle */
  "user_text",            /* TXXX ID3FID_USERTEXT,          *< User defined text information */
  "year",                 /* TYER ID3FID_YEAR,              *< Year */
  "ufid",                 /* UFID ID3FID_UNIQUEFILEID,      *< Unique file identifier */
  "terms",                /* USER ID3FID_TERMSOFUSE,        *< Terms of use */
  "lyrics",               /* USLT ID3FID_UNSYNCEDLYRICS,    *< Unsynchronized lyric/text transcription */
  "www_comercial",        /* WCOM ID3FID_WWWCOMMERCIALINFO, *< Commercial information */
  "www_copyright",        /* WCOP ID3FID_WWWCOPYRIGHT,      *< Copyright/Legal infromation */
  "www_audiofile",        /* WOAF ID3FID_WWWAUDIOFILE,      *< Official audio file webpage */
  "www_artist",           /* WOAR ID3FID_WWWARTIST,         *< Official artist/performer webpage */
  "www_audiosource",      /* WOAS ID3FID_WWWAUDIOSOURCE,    *< Official audio source webpage */
  "www_radio",            /* WORS ID3FID_WWWRADIOPAGE,      *< Official internet radio station homepage */
  "www_payment",          /* WPAY ID3FID_WWWPAYMENT,        *< Payment */
  "www_publisher",        /* WPUB ID3FID_WWWPUBLISHER,      *< Official publisher webpage */
  "www_user",             /* WXXX ID3FID_WWWUSER,           *< User defined URL link */
  "metacrypto",           /*      ID3FID_METACRYPTO,        *< Encrypted meta frame (id3v2.2.x) */
  "metacompression"       /*      ID3FID_METACOMPRESSION,   *< Compressed meta frame (id3v2.2.1) */
};
BOOST_STATIC_ASSERT(sizeof(frame_lookup)/sizeof(const char*) == ID3FID_LASTFRAMEID);

const char* lookup_field(ID3_FieldID id)
{
  if (id < ID3FN_LASTFIELDID)
    return id_lookup[id];
  else
    return "unknown";
}

const char* lookup_frame(ID3_FrameID id)
{
  if (id < ID3FID_LASTFRAMEID)
    return frame_lookup[id];
  else
    return "unknown";
}


template<class CharT, class traits>
std::basic_ostream<CharT,traits>& operator<<(std::basic_ostream<CharT,traits>& stream, ID3_FieldID id)
{
  stream << lookup_field(id);
  return stream;
}

template<class CharT, class traits>
std::basic_ostream<CharT,traits>& operator<<(std::basic_ostream<CharT,traits>& stream, ID3_FrameID id)
{
  stream << lookup_frame(id);
  return stream;
}


void set_id3_metadata(const char* path,
                      const char* frame,
                      int count,
                      const char* field,
                      const char* data)
{
  std::cout << "M 644 inline " << "files/" << path 
            << "/id3/" << frame << '/' << count << '/' << field
            << '\n'
            << "data " << std::strlen(data) << '\n'
            << data << '\n';
}

void set_main_metadata(const char* path,
                       const char* type,
                       const char* data)
{
  std::cout << "M 644 inline " << "files/" << path << "/"
            << type << "\n"
            << "data " << std::strlen(data) << "\n"
            << data << "\n";
}

void set_sort_metadata(const char* path,
                       const char* sort_type,
                       const char* sort_data)
{
  std::cout << "C \"" << "files/" << path << "\" "
            << "sort1/" 
            << sort_type << '/';

  if (strlen(sort_data) > 0)
    std::cout << sort_data << '\n';
  else
    std::cout << "Unknown" << '\n';
}

void set_sort_metadata(const char* path,
                       const char* sort_type_1,
                       const char* sort_data_1,
                       const char* sort_type_2,
                       const char* sort_data_2)
{
  std::cout << "C \"" << "files/" << path << "\" "
            << "sort2/" 
            << sort_type_1 << '/'
            << sort_type_2 << '/';

  if (strlen(sort_data_1) > 0)
    std::cout << sort_data_1 << '/';
  else
    std::cout << "Unknown" << '/';

  if (strlen(sort_data_2) > 0)
    std::cout << sort_data_2 << '\n';
  else
    std::cout << "Unknown" << '\n';
}

void set_sort_metadata(const char* path,
                       const char* sort_type_1,
                       const char* sort_data_1,
                       const char* sort_type_2,
                       const char* sort_data_2,
                       const char* sort_type_3,
                       const char* sort_data_3)
{
  std::cout << "C \"" << "files/" << path << "\" "
            << "sort3/" 
            << sort_type_1 << '/'
            << sort_type_2 << '/'
            << sort_type_3 << '/';

  if (strlen(sort_data_1) > 0)
    std::cout << sort_data_1 << '/';
  else
    std::cout << "Unknown" << '/';

  if (strlen(sort_data_2) > 0)
    std::cout << sort_data_2 << '/';
  else
    std::cout << "Unknown" << '/';

  if (strlen(sort_data_3) > 0)
    std::cout << sort_data_3 << '\n';
  else
    std::cout << "Unknown" << '\n';
}

struct PrintMetadata
{
  PrintMetadata(size_t strip):strip_(strip){}

  size_t strip_;

  void operator()(const fs::path& path)const
  {
    if (path.extension() != ".mp3")
      return;

    ID3_Tag tag(path.string().c_str());
    const char* stripped_path = path.string().c_str() + strip_;

    boost::array<int, ID3FID_LASTFRAMEID> frame_count;
    std::fill(frame_count.begin(),frame_count.end(),0);

    ID3_Tag::Iterator* tag_iterator = tag.CreateIterator();
    if (!tag_iterator)
      return;

    const char* artist = " ";
    const char* album  = " ";
    const char* title  = " ";
    for(ID3_Frame* frame = tag_iterator->GetNext();
        frame;
        frame = tag_iterator->GetNext())
    {
      ID3_Frame::Iterator* frame_iterator = frame->CreateIterator();
      if (!frame_iterator)
        break;

      ID3_FrameID frame_id = frame->GetID();
      if (frame_id >= ID3FID_LASTFRAMEID)
        break;

      const char* frame_id_string = lookup_frame(frame_id);

      for(ID3_Field* field = frame_iterator->GetNext();
          field;
          field = frame_iterator->GetNext())
      {
        bool have_text = false;
        if (field->GetType() == ID3FTY_TEXTSTRING)
        {
          if (!have_text)
            have_text = true;
          else
            have_text = true;
          const char* field_id_string = lookup_field(field->GetID());
          const char* field_data = field->GetRawText();
          if (!field_data || !field_id_string)
            break;

          set_id3_metadata(stripped_path,
                           frame_id_string,
                           frame_count[frame_id],
                           field_id_string,
                           field_data);

          if (field_data && field_data[0])
          {
            if (frame_id == ID3FID_LEADARTIST)
              artist = field_data;
            else if (frame_id == ID3FID_ALBUM)
              album  = field_data;
            else if (frame_id == ID3FID_TITLE)
              title  = field_data;
          }
        }
      }
      frame_count[frame_id]++;
      //delete frame_iterator;
    }
    std::replace(const_cast<char*>(title), const_cast<char*>(title + strlen(title)), '/', '_');
    std::replace(const_cast<char*>(artist), const_cast<char*>(artist + strlen(artist)), '/', '_');
    std::replace(const_cast<char*>(album), const_cast<char*>(album + strlen(album)), '/', '_');
    //delete tag_iterator;
    set_main_metadata(stripped_path,
                      "artist",artist);
    set_main_metadata(stripped_path,
                      "album",album);
    set_main_metadata(stripped_path,
                      "title",title);
    set_sort_metadata(stripped_path,
                      "artist",artist,
                      "title",title);
    set_sort_metadata(stripped_path,
                      "artist",artist,
                      "album",album,
                      "title",title);

  }
};

QSMPINDEXER_END

int main(int argc, char* argv[])
{

  po::options_description options;
  options.add_options()
    ("help", "produce help message")
    ("directory", po::value<std::string>(), "directory to index");
  po::positional_options_description positional_options;
  positional_options.add("directory",1);

  po::variables_map arg_map;
  po::store(po::command_line_parser(argc,argv)
              .options(options)
              .positional(positional_options)
              .run(),
            arg_map);

  if (arg_map.count("help") != 0 ||
      arg_map.count("directory") == 0)
  {
    std::cerr << "Usage: qsmp_indexer [options] [directory]\n"
              << options;
    return 1;
  }

#ifdef WIN32
  _setmode(_fileno(stdout),_O_BINARY);
#endif

  std::string commit_message("Initial index.");  
  std::cout << "reset refs/heads/master\n"
            << "commit refs/heads/master\n"
            << "committer qsmp_indexer <qsmp_indexer@example.com> now\n"
            << "data " << commit_message.size() << "\n"
            << commit_message << "\n";

  std::string directory = arg_map["directory"].as<std::string>();
  size_t strip = directory.size();

  fs::recursive_directory_iterator dir_iter(arg_map["directory"].as<std::string>());
  std::for_each(dir_iter,
                fs::recursive_directory_iterator(),
                qsmp_indexer::PrintMetadata(strip));

  return 0;
}

