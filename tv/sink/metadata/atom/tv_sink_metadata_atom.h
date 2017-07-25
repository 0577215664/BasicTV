#ifndef TV_SINK_METADATA_ATOM_H
#define TV_SINK_METADATA_ATOM_H
#include "../tv_sink_metadata.h"
extern TV_SINK_METADATA_INIT(atom);
extern TV_SINK_METADATA_CLOSE(atom);
extern TV_SINK_METADATA_UPDATE(atom);

/*
  All URLs that Atom provides are for a generic HTTP container that can generate
  the proper item to download from the filename (once that is implemented).

  TODO: bind a metadata sink to one or more sinks, and allow lookups and allow
  for sane and standardized filenames by converying the filename information
  as a binding to a tv_item_t (to prevent malicious or unintentional duplicates)

  Not only files for the HTTP or whole-file ones, but also allow for binding
  live streams (which we should be bound to serve through some setting) to Atom
  feed metadata, and allow for opening that in whatever.
*/

/*
  For simplicity at the moment, the Atom feed is sent over it's own port.
  In the future, I would like to see one TCP socket (80) responsible for
  interfacing with all of this jazz, but the idea of multiple states could
  hinder that ability (even though in the case of Atom, that doesn't really
  make too much sense)
*/

struct tv_sink_metadata_atom_state_t{
	id_t_ socket_id = ID_BLANK_ID;
};
#endif
