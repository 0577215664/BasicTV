# Changelog


## (unreleased)

### Other

* Fixed forms (setting request_payload properly) [Daniel Hoffman]

* Fixed HTTP stuff, again. [Daniel Hoffman]

* Fixed HTTP stuff again. [Daniel Hoffman]

* Fixed HTTP. [Daniel Hoffman]

* More work on HTTP, connects, but sends an invalid response. [Daniel Hoffman]

* Compilable HTTP system, not tested to work. [Daniel Hoffman]

* More HTTP work. [Daniel Hoffman]

* Started to redefine HTTP packet data to allow for easy bidirectional file transfer. [Daniel Hoffman]

* More HTML stuff. [Daniel Hoffman]

* Fixed error in HTML forms (sink-item to sink-item-window bindings) [Daniel Hoffman]

* Beautified HTML. [Daniel Hoffman]

* More macro function declarations, fixed HTTP GET parser. [Daniel Hoffman]

* Finished initial HTML responses, now to write everything else. [Daniel Hoffman]

* More HTML frontpage frontend code, some backend parsing. [Daniel Hoffman]

* Made on the fly parsing of a template string, reimplemented HTML as macro functions, pretty nice. [Daniel Hoffman]

* Fixed segfaulting on init. [Daniel Hoffman]

* Fixed disk cache not being updated. [Daniel Hoffman]

* Fixed disk exporting and other tier nonsense. [Daniel Hoffman]

* Fixed bug where default values for id_tier_state_t were valid, polishing up and ran a test. [Daniel Hoffman]

* Moved ID tiers from inline request filling to a buffer to prevent hangs with networking. [Daniel Hoffman]

* Small stuff. [Daniel Hoffman]

* Completely removed old protocol networking code, moving over to net_interface and ID tiers. [Daniel Hoffman]

* More ID tier network logic, simplified indexing system implemented (partially) [Daniel Hoffman]

* Start network integration as an ID tier. [Daniel Hoffman]

* Prepare id ties for network integration. [Daniel Hoffman]

* ID request medium abstraction and stats functions. [Daniel Hoffman]

* Marginal improvements, compiles. [Daniel Hoffman]

* More deleting old code, offloading audio prop code to generic struct. [Daniel Hoffman]

* Moving computers. [Daniel Hoffman]

* Numerical tests. [Daniel Hoffman]

* Fixed strip_to_transportable. [Daniel Hoffman]

* Fixed transport bug with IDs. [Daniel Hoffman]

* Temp. disabled strip_to_transportable. [Daniel Hoffman]

* Few smalls things, added pragma warning. [Daniel Hoffman]

* More ID work, might work fine this time. [Daniel Hoffman]

* Few fixes to ID transport. [Daniel Hoffman]

* Marginal improvement stuff. [Daniel Hoffman]

* Removed old video code, to reimplement later. [Daniel Hoffman]

* Thought I lost this commit. [Daniel Hoffman]

* Marginally more progress on ID transport front. [Daniel Hoffman]

* Added some tests for core ID transport functions, and what was included passed. [Daniel Hoffman]

* Recommented broke code, make core transport tests. [Daniel Hoffman]

* Whoops, forgot new files. [Daniel Hoffman]

* More transport jargon, should work in concept at least. [Daniel Hoffman]

* Haven't even ran yet, won't work, shifting to a more flexible permissions system. [Daniel Hoffman]

* Removed unused function no warning flag. [Daniel Hoffman]

* Fixed all warnings, still have pragma messages. [Daniel Hoffman]

* Oh darn. [Daniel Hoffman]

* Fix. [Daniel Hoffman]

* Fixed typo in debugging stuff. [Daniel Hoffman]

* Forgot to move over to staging, bugfixing. [Daniel Hoffman]

* Fixed one warning. [Daniel Hoffman]

* Finished but simplified disk exporting. [Daniel Hoffman]

* Added personal.gpg (blank) [Daniel Hoffman]

* Re-enabled disabling audio, OGG implemented. [Daniel Hoffman]

* More bug fixing. [Daniel Hoffman]

* Added file support in transcode, simpler interface. [Daniel Hoffman]

* Just converted http code over to a formal constructor. [Daniel Hoffman]

* Maybe this'll help. [Daniel Hoffman]

* ARM fixes. [Daniel Hoffman]

* More ARM fixes. [Daniel Hoffman]

* Tv_sink attempted fix for ARM. [Daniel Hoffman]

* Store HTTP GET stuff, removed SDL_mixer references. [Daniel Hoffman]

* Added downloading basics. [Daniel Hoffman]

* Fixed HTTP stuff. [Daniel Hoffman]

* Even more marginal HTTP work. [Daniel Hoffman]

* Updated and marginally cleaned up HTTP code. [Daniel Hoffman]

* More HTTP work. [Daniel Hoffman]

* Marginally more work. [Daniel Hoffman]

* Tied togethter more Atom code. [Daniel Hoffman]

* Pushed tv_menu files. [Daniel Hoffman]

* Started conversion over to Vorbis comments for internal use. [Daniel Hoffman]

* Simplified and expanded macro-based getters and setters, more Atom code. [Daniel Hoffman]

* Merge branch 'master' into staging (forgot to switch branches) [Daniel Hoffman]

* Added Atom stuff to http file driver. [Daniel Hoffman]

* Finished moving atom to http file driver, not tv metadata sink. [Daniel Hoffman]

* Removed output_file__ [Daniel Hoffman]

* Net http file drivers. [Daniel Hoffman]

* Added std::vector::reserve to callback. [Daniel Hoffman]

* Numerical TCP broadcasting. [Daniel Hoffman]

* Store types as vectors, send out buik type requests too. [Daniel Hoffman]

* Numerical streaming works somewhat well. [Daniel Hoffman]

* Console code to abstract playing to sinks, allowing for inbound recordings and live broadcasting (in theory) [Daniel Hoffman]

* Numerical streams are available in the console (not tested), audio is guaranteed to work after sink mechanics change. [Daniel Hoffman]

* Listed TCP accept in sink_medium_vector. [Daniel Hoffman]

* Added net ip information to TCP sinks. [Daniel Hoffman]

* Added stripping currently-unused unit from TCP sink. [Daniel Hoffman]

* Better framing system for numerical data. [Daniel Hoffman]

* Numerical stream support. [Daniel Hoffman]

* Fixed blank passing in add_data_to_state. [Daniel Hoffman]

* Updated print_backtrace and print_soe. [Daniel Hoffman]

* Search for already decoded chunks in add_data_to_state, starting from simplest to hardest. [Daniel Hoffman]

* Optimized escape code, remoevd redundancy checks in get_id_ptr. [Daniel Hoffman]

* Use std::swap instead of XOR for convert::nbo::to. [Daniel Hoffman]

* Removed sleeps in recv_to_buffer and send_to_buffer. [Daniel Hoffman]

* Fixed segfaulting on invalid connection. [Daniel Hoffman]

* Remove some more spam prints. [Daniel Hoffman]

* Cleaned up connection code, delete peer if we can't connect. [Daniel Hoffman]

* Merged master to staging. [Daniel Hoffman]

* Implemented memory tier lookups as a type-based jump table. [Daniel Hoffman]

* Removed some common errors with no-prints. [Daniel Hoffman]

* Fewer prints. [Daniel Hoffman]

* Fewer print statements. [Daniel Hoffman]

* Fixed zstd deletion stuff. [Daniel Hoffman]

* Check Pa_StreamStart, fix compression level scaling for zstd. [Daniel Hoffman]

* Fixed typo with LDLIBS. [Daniel Hoffman]

* Fixed deadlock with re-listing tier 0 state. [Daniel Hoffman]

* Fixed redundant ID listing. [Daniel Hoffman]

* Merge branch 'staging' [Daniel Hoffman]

* Merge branch 'staging' of github.com:Dako300/BasicTV. [Daniel Hoffman]

* Merge branch 'staging' [Daniel Hoffman]

* Removed un-needed prints. [Daniel Hoffman]

* More HTTP work, gave up on operator overloading for now. [Daniel Hoffman]

* Terrible commit, moving over for ioa. [Daniel Hoffman]

* More HTTP code. [Daniel Hoffman]

* HTTP file listing system, no HTTP code yet. [Daniel Hoffman]

* Formal type designation and small Atom stuff. [Daniel Hoffman]

* Added metadata skeleton and Atom 1.0 headers. [Daniel Hoffman]

* Removed debug flags in standard CXXFLAGS. [name]

* Prevented automatic loading to all tiers. [Daniel Hoffman]

* Attempt to fix multiple listings in ID_TIER_CACHE_GET. [Daniel Hoffman]

* Combined two makefiles. [Daniel Hoffman]

* New Makefile. [Daniel Hoffman]

* Temp. disabled disk to allow for tier shift debugging. [Daniel Hoffman]

* Strip_to_only_rules just returns the data, OK for now since everything is deleted from all tiers and data is only exported at the end of a cycle (no tmp.) [Daniel Hoffman]

* Re-enabled should_export, debugging with tier management. [Daniel Hoffman]

* Fixed re-loading private keys (weird calls before) [Daniel Hoffman]

* Removed un-needed prints. [Daniel Hoffman]

* Merged staging, slightly tested disk support, sink system. [Daniel Hoffman]

* Don't export non-exportable data metadata. [Daniel Hoffman]

* More tiered work, looks fine to me. [Daniel Hoffman]

* Fixed strip_to_only_rules (i think) [Daniel Hoffman]

* Stuff. [Daniel Hoffman]

* Beginnings of a tier shfiting system and disk writing, doesn't properly work yet. [Daniel Hoffman]

* Fixed blank ID setting. [Daniel Hoffman]

* Updated window to prevent infinitely-growing linked list length. [Daniel Hoffman]

* Resorted tv loop to request the full tv_forward_buffer. [Daniel Hoffman]

* Added retval. [Daniel Hoffman]

* Only create type requests when we have active sockets. [Daniel Hoffman]

* Fixed Makefile. [Daniel Hoffman]

* Forgot fast tag. [Daniel Hoffman]

* Only make type requests to currently connected peers, allowed for an optimized build. [Daniel Hoffman]

* Fixed segfault on no more forward linked list data. [Daniel Hoffman]

* Proper tv_forward_buffer enforcement and fetching more IDs. [Daniel Hoffman]

* Checks against attempts to decode non-existant frames, but still catalog them. [Daniel Hoffman]

* Setting for chunk size. [Daniel Hoffman]

* New audio sink system, still some clicking, but sounds better. [Daniel Hoffman]

* Stuff. [Daniel Hoffman]

* Continue on null in tv_audio_mixer, not print a warning. [Daniel Hoffman]

* Fixed console disconnect crash. [Daniel Hoffman]

* Re-enabled full network buffer. [Daniel Hoffman]

* Fixed typo in settings, disabled sorting on every get (too slow) [Daniel Hoffman]

* Reverted attampted fix for better network performance, finally optimized settings. [Daniel Hoffman]

* Need to review my work more. [Daniel Hoffman]

* Tv_window fix. [Daniel Hoffman]

* Fixed typos. [Daniel Hoffman]

* Fixed typo in tv item gen. [Daniel Hoffman]

* Fix typo. [Daniel Hoffman]

* State codec stuff. [Daniel Hoffman]

* Lots of work on TV sink system. [Daniel Hoffman]

* Fix for broken socket in sending code. [Daniel Hoffman]

* Darn retvals. [Daniel Hoffman]

* Other stuff. [Daniel Hoffman]

* Fixed SDL_mixer bug. [Daniel Hoffman]

* Re-enabled compression. [Daniel Hoffman]

* Initial, broken, new audio playback system. [Daniel Hoffman]

* Continue on null in tv_audio_mixer, not print a warning. [Daniel Hoffman]

* Fixed console disconnect crash. [Daniel Hoffman]

* Re-enabled full network buffer. [Daniel Hoffman]

* Fixed typo in settings, disabled sorting on every get (too slow) [Daniel Hoffman]

* Reverted attampted fix for better network performance, finally optimized settings. [Daniel Hoffman]

* Need to review my work more. [Daniel Hoffman]

* Probably did something. [Daniel Hoffman]

* Fixed typos. [Daniel Hoffman]

* Fixed typo in tv item gen. [Daniel Hoffman]

* Fix typo. [Daniel Hoffman]

* State codec stuff. [Daniel Hoffman]

* Fix for broken socket in sending code. [Daniel Hoffman]

* Darn retvals. [Daniel Hoffman]

* Other stuff. [Daniel Hoffman]

* Re-enabled compression. [Daniel Hoffman]

* Fixed SDL_mixer bug. [Daniel Hoffman]

* Fixed typo making redundant id requests. [Daniel Hoffman]

* Removed print statement in tv_frame_scroll_to_time. [Daniel Hoffman]

* Updated add_data rules for ID linked list. [Daniel Hoffman]

* Improved linked listing with tv frames. [Daniel Hoffman]

* Choice between libao and SDL_mixer. [Daniel Hoffman]

* Switched over to SDL_mixer again. [Daniel Hoffman]

* Cleanup and better console interface for tv_manager. [Daniel Hoffman]

* Disabled flushing to std::cout, fixed crazy proto id request print. [Daniel Hoffman]

* Forgot test_audio.cpp. [Daniel Hoffman]

* Fixed audio playback. [Daniel Hoffman]

* Cleanup and fix console exit segfault. [Daniel Hoffman]

* Fix typo. [Daniel Hoffman]

* New stuff. [Daniel Hoffman]

* Sanitize ID inputs for id::from_hex. [Daniel Hoffman]

* Dumb internet. [Daniel Hoffman]

* Tv_manager live fix. [Daniel Hoffman]

* Works. [Daniel Hoffman]

* Cleanup some beyond useless print statements. [Daniel Hoffman]

* Fixes. [Daniel Hoffman]

* Async TCP. [Daniel Hoffman]

* Re-added disabling audio. [Daniel Hoffman]

* Re-added SDL_Mixer because I thought libao was broken, it wasn't. [Daniel Hoffman]

* Audio transmission and receiving works, but audio is garbled and low pitched (but discernible) on both. [Daniel Hoffman]

* Allow prefetching next frame in tv_frame_scroll_to_time. [Daniel Hoffman]

* Added tv_item_t reader macro function. [Daniel Hoffman]

* Better stats output, looks like networking is working. [Daniel Hoffman]

* Updated README.md. [Daniel Hoffman]

* ID requests are almost working, garbled though. [Daniel Hoffman]

* Fixed segfault with escape_vector/unescape_vector. [Daniel Hoffman]

* Fix ID sets. [Daniel Hoffman]

* Fixed escaping. [Daniel Hoffman]

* Fixed force_to_extra and added tier 0 metadata. [Daniel Hoffman]

* Organized tier 0/RAM specific code, helps deadlocks a lot. [Daniel Hoffman]

* Prevent id_tier_state_t from being deleted, fixed typos. [Daniel Hoffman]

* Added cache tier, no real testing done yet. [Daniel Hoffman]

* Properly update entire cache on production priv key bootstrap. [Daniel Hoffman]

* Fixed production id bootstrapping issues, simplified. [Daniel Hoffman]

* Forgot some noisy prints. [Daniel Hoffman]

* Refactored for tier, looks fine now. [Daniel Hoffman]

* Lots of storage tier stuff, but deadlocks. [Daniel Hoffman]

* Last successful build for a while, gutting ID lookups. [Daniel Hoffman]

* More id_tier stuff, putting this on hold for now. [Daniel Hoffman]

* More tier work. [Daniel Hoffman]

* Tier stuff and new testing system. [Daniel Hoffman]

* Probably did something worth commiting. [Daniel Hoffman]

* Deleted all ID vector transport, replaced with one byte ID sets. [Daniel Hoffman]

* Marginal improvement, re-fixed tests. [Daniel Hoffman]

* Organized requests, no more infinite growth. [Daniel Hoffman]

* Changed stats to cached IDs breakdown by size. [Daniel Hoffman]

* Count pending connection requests as sockets. [Daniel Hoffman]

* Fixed typo with requests imediately being deleted. [Daniel Hoffman]

* Fixed really bad judgement in networking. [Daniel Hoffman]

* Fixed ID request deadlock. [Daniel Hoffman]

* More networking fixes. [Daniel Hoffman]

* Improvements on connectivity reliability. [Daniel Hoffman]

* Added node script. [Daniel Hoffman]

* Updated README.md. [Daniel Hoffman]

* Replaced SDL_audio with libao, actually works. [Daniel Hoffman]

* Cleanup. [Daniel Hoffman]

* Attempt to make a callback system work. [Daniel Hoffman]

* Added connect.sh to actually connect to the network. [Daniel Hoffman]

* Test connections over the Tor network. [Daniel Hoffman]

* Temp disabled tests. [Daniel Hoffman]

* Fixed typo with math number adding, still a bad impl. [Daniel Hoffman]

* Added byte vector vector importing. [Daniel Hoffman]

* Merge pull request #3 from Dako300/net_interface. [Daniel]

  Net interface

* Fixed networking again, implemented P_UNABLE, other stuff. [Daniel Hoffman]

* Readded networking script, works reasonably well. [Daniel Hoffman]

* Started converting some code over to net_interface_ip_address_t. [Daniel Hoffman]

* A bit more networking work. [Daniel Hoffman]

* Updated README.md for packet radio writeup. [Daniel Hoffman]

* Added TCP IP recv code, decongested headers. [Daniel Hoffman]

* Spread out the networking jargon into far more files, little bit more TCP. [Daniel Hoffman]

* Added some software getters and setters, clarifications, etc. [Daniel Hoffman]

* Updated README. [Daniel Hoffman]

* More networking interface work. [Daniel Hoffman]

* Little bit everywhere, generic network improvements, adding onto number support. [Daniel Hoffman]

* Updated audio tests for frame encoding and decoding. [Daniel Hoffman]

* Fixed opus compression again. [Daniel Hoffman]

* Changed all sample references to be 1D, passed codec sets and sample sets as ptrs. [Daniel Hoffman]

* Opus decoding works. [Daniel Hoffman]

* Added and changed some opus stuff, made a formal test for audio codecs. [Daniel Hoffman]

* Fixed some exporting glitches, made a crude audio test. [Daniel Hoffman]

* Playing static, we're getting closer... [Daniel Hoffman]

* Updated README.md. [Daniel Hoffman]

* Still get static, but with new and improved codebase and verbosity! [Daniel Hoffman]

* At least I get static. [Daniel Hoffman]

* Converted to a more flexible exporting and ID forwarding rule system. [Daniel Hoffman]

* Added live time for tv_test_audio, only need state decoding across multiple calls to test. [Daniel Hoffman]

* Added generic abstraction for IP netwoking, allowing for future AX.25 dev. [Daniel Hoffman]

* Cleaned up and standardized tv_transcode stuff, two dimensional vectors for codecs. [Daniel Hoffman]

* Transcoding abstractions updated for multi-packet payloads partially, Opus loading from file via console looks fine. [Dan H]

* Added basic transcoding template for future codecs. [Dan H]

* Started making a better UI for the console (tv_manager) [Daniel Hoffman]

* Fixed one compiler bug on RPi. [name]

* Added escrow folder, basic types, and comments. [Daniel Hoffman]

* Added more error flags, made code more standards compliant. [Daniel Hoffman]

* Fixed type and ID requests, two nodes are requesting and filling in tests, problem with redundancy. [Daniel Hoffman]

* Updated README.md. [Daniel Hoffman]

* Fixed little places here and there. [Daniel Hoffman]

* Fixed offset problem with id_api::raw, mod_inc pulling works fine now. [Daniel Hoffman]

* Added cleaning up id_api::add_data new data on failure. [Daniel Hoffman]

* Fixed ptr_id returning null on non-type-specific types. [Daniel Hoffman]

* Updated FORMATTING.txt. [Daniel Hoffman]

* Fixed all warnings, should keep it this way forever. [Daniel Hoffman]

* Fixed locks, added threads.cpp, deadlock detection in ID lookups, misc networking. [Daniel Hoffman]

* Listed IDs in type cache on raw cache input (fixing peer recognition problems) [Daniel Hoffman]

* Organized loop code into seperate file. [Daniel Hoffman]

* Fixed double-free in net proto socket test. [Daniel Hoffman]

* Forced all tests to clean up after themselves. [Daniel Hoffman]

* Updated README.md a little bit. [Daniel Hoffman]

* Cleaned up and standardized print output. [Daniel Hoffman]

* Worked on routine requests, sending them works okay-ish-ly too between five versions. [Daniel Hoffman]

* Updated TODO.txt. [Daniel Hoffman]

* Improved network request datatypes and created generic handlers for sending. [Daniel Hoffman]

* Allowed adding IDs to id_cache and checking it on searches. [Daniel Hoffman]

* Cleaned up useless print messages. [Daniel Hoffman]

* Fixed escape stuff, should work fine now. [Daniel Hoffman]

* Fixed net_proto_socket_t fragmenting payload (probably) [Daniel Hoffman]

* Made an independent proto_socket test, it's on the fritz, but it can work. [Daniel Hoffman]

* Worked on number sets, made size on disk sustainable after multiple runs. [Daniel Hoffman]

* Re-added test functions (commented for speeding up ID set testing) [Daniel Hoffman]

* Fixed id_set transport, everything is looking nice. [Daniel Hoffman]

* Fixed naming problem, cleaned up needlessly verbose start sequence, discovered id_set is broken. [Daniel Hoffman]

* Somewhat formal formatting guide. [Daniel Hoffman]

* Fixed README.md typos. [Daniel Hoffman]

* Fixed minor numbers problem with common sense. [Daniel Hoffman]

* Fixed major species number transport, minor is still broken. [Daniel Hoffman]

* Configuring basics for tv_item_t (moving away from continuous stream model, towards content blocks) [Daniel Hoffman]

* Slightly fixed minor number tv_frame_number transporting. [Daniel Hoffman]

* Added tv_frame_numbers test. [Daniel Hoffman]

* Updated README.md. [Daniel Hoffman]

* Tv frame number initial configuration, multivector export (no import) [Daniel Hoffman]

* Simplified fetch_* functions. [Daniel Hoffman]

* Split main.cpp into init, close, and test files. optimized data_id_t::is_owner. [Daniel Hoffman]

* Added type byte to ID and fixed transport issues. [Daniel Hoffman]

* Shoe-horned a new disk management system, barely works, will fix later. [Daniel Hoffman]

* Moved ID exporting over to individual disk index types. [Daniel Hoffman]

* Fixed holes in type conversion tables. [Daniel Hoffman]

* Moved over to one byte type ids. [Daniel Hoffman]

* Added individual exportability and changed scope of some vars. [Daniel Hoffman]

* Fixed ID exporting rules. [Daniel Hoffman]

* Worked on networking, finally coming together a bit. [Daniel Hoffman]

* More misc networking magic. [Daniel Hoffman]

* Added zstd compression. [Daniel Hoffman]

* Escape tested to work (simple tests) [Daniel Hoffman]

* Updated README.md to new GitHub parser. [Daniel Hoffman]

* Don't load an entire data chunk to check for ID. [Daniel Hoffman]

* Marginally better type request sending system. [Daniel Hoffman]

* Implemented bare_recv. [Daniel Hoffman]

* Removed raise. [Daniel Hoffman]

* Changed status of spammed print statements. [Daniel Hoffman]

* Included data functions in net_proto_loop. [Daniel Hoffman]

* Taking a stab at exporting ID data. [Daniel Hoffman]

* Delete type requests upon filling. [Daniel Hoffman]

* Send net_proto_peer_t on connect. [Daniel Hoffman]

* Try catch in socket sending. [Daniel Hoffman]

* Added type request sending. [Daniel Hoffman]

* Fixed filling my own type requests. [Daniel Hoffman]

* Referenced routine requests. [Daniel Hoffman]

* Removed currently broken bootstrap list, relies no settings.cfg. [Daniel Hoffman]

* Commented out unfinished TCP holepunch function. [Daniel Hoffman]

* Removed used connection requests. [Daniel Hoffman]

* Fixed stat_sample_set_t spamming with net_socket_t. [Daniel Hoffman]

* Forgot to open a listening port. [Daniel Hoffman]

* Even more verbose network prints. [Daniel Hoffman]

* Removed too verbose messages. [Daniel Hoffman]

* Added removal of stale con_req. [Daniel Hoffman]

* Stopped bug where force escape corrupted data. [Daniel Hoffman]

* Network connection cleanup, verbose messages. [Daniel Hoffman]

* Fix encryption API requesting data. [Daniel Hoffman]

* Progress bar for encryption benchmark. [Daniel Hoffman]

* Loop checking prevent fp exception in audio. [Daniel Hoffman]

* Can disable audio and video from command line. [Daniel Hoffman]

* Changed ARM build error. [Daniel Hoffman]

* Exportable benchmarks. [Daniel Hoffman]

* Added AES test and small clean ups. [Daniel Hoffman]

* More work on AES-192 and SHA-256 encryption. [Daniel Hoffman]

* Fix for infinite closing loop. [Daniel Hoffman]

* Broken export system, in progress. [Daniel Hoffman]

* Added initial AES-256 wrapper code. [Daniel Hoffman]

* Improved, working, and testable RSA OpenSSL wrapper set. [Daniel Hoffman]

* Can now encrypt more than modulus length in RSA. [Daniel Hoffman]

* Partially fixed encryption. [Daniel Hoffman]

* Stats cleanup. [Daniel Hoffman]

* Added network socket decryption (partial) [Daniel Hoffman]

* Reworked IP address storage to require less informaiton at start. [Daniel Hoffman]

* Cleaned up source a bit. [Daniel Hoffman]

* Initial peer to peer connection code. [Daniel Hoffman]

* Fixed the first person speak. [Daniel Hoffman]

* Fixed network stats bug. [Daniel Hoffman]

* Quick fix. [Daniel Hoffman]

* Forgot one paragraph in README.md. [Daniel Hoffman]

* Rewrote README.md. [Daniel Hoffman]

* Polished up network requests. [Daniel Hoffman]

* Improved send logic, dedicated file to network protocol API. [Daniel Hoffman]

* More well standardize and implement network requests. [Daniel Hoffman]

* More converting over to new requests. [Daniel Hoffman]

* Start of new request sytem. [Daniel Hoffman]

* Removed redundant comments in settings file and made it reflect current type request model. [Daniel Hoffman]

* Actually use the previos comment variable for blacklists. [Daniel Hoffman]

* Created settings for number of peers to query for white and blacklists. [Daniel Hoffman]

* Some basic network stats structure is down. [Daniel Hoffman]

* Removed WAV files from Git tree. [Daniel Hoffman]

* Console functional enough to load audio and play it back. [Daniel Hoffman]

* Fancy tables (and id_api_get_all) [Daniel Hoffman]

* Updated .gitignore. [Daniel Hoffman]

* Smart table generator doesn't render all blank columns. [Daniel Hoffman]

* More commands. [Daniel Hoffman]

* Added more commands. [Daniel Hoffman]

* Added more commands. [Daniel Hoffman]

* Possible console set_req_const fix. [Daniel Hoffman]

* Possible console set_req_const fix. [Daniel Hoffman]

* Merge branch 'ascent12-master' [Daniel Hoffman]

  Makefile runs a lot faster, thanks

* Added new Makefile. [Daniel Hoffman]

* Updated build system. [Scott Anderson]

* Added missing #includes. [Scott Anderson]

* Changed #includes to use <> for system headers. [Scott Anderson]

* Fixed ID hex transport. [Daniel Hoffman]

* Fixed non-delisting with unknown types, console fixes. [Daniel Hoffman]

* Basic console configuration. [Daniel Hoffman]

* Cleaned up id hex transport, hyphen between UUID and hash. [Daniel Hoffman]

* Made data_folder a setting. [Daniel Hoffman]

* Initial Opus work. [Daniel Hoffman]

* Encryption private key transport and production ID transport works. [Daniel Hoffman]

* A little less broken every day. [Daniel Hoffman]

* Transport works without NBO, weird. [Daniel Hoffman]

* Forgot IRC was already in there. [Daniel Hoffman]

* ID transport stuff. [Daniel Hoffman]

* Included IRC in README.md. [Daniel Hoffman]

* Almost made it work, just need add_data finished. [Daniel Hoffman]

* Still rough, but can export when directory is premade. [Daniel Hoffman]

* Merge branch 'master' of github.com:Dako300/BasicTV. [Daniel Hoffman]

  I said so

* Merge pull request #1 from nvllvs/master. [Daniel]

  Add dependency installation script

* Add dependency installation script. [nvllvs]

* Cleaned up a bit, cant export, broken. [Daniel Hoffman]

* Converted a lot over to id_t_ [Daniel Hoffman]

* Fixed a lot of warnings. [Daniel Hoffman]

* Made the test frame match the supplied window resolution. [Daniel Hoffman]

* Test card test. [Daniel Hoffman]

* Done with endian macro fix. [Daniel Hoffman]

* Hoepful fix, but forces 24-bit color. [Daniel Hoffman]

* Fixed SPARC typo. [Daniel Hoffman]

* Cleaned up object files. [Daniel Hoffman]

* Misc fixes. [Daniel Hoffman]

* Fixed typo. [Daniel Hoffman]

* Drawing to screen fix for SPARC. [Daniel Hoffman]

* Debugging stuff for SPARC machine. [Daniel Hoffman]

* Assumes the correct endian. [Daniel Hoffman]

* Attempted fix for SPARC64. [Daniel Hoffman]

* Removed rsa encryption test as default. [Daniel Hoffman]

* Fixed audio for MSB. [Daniel Hoffman]

* Fixed typo. [Daniel Hoffman]

* Fixed typo. [Daniel Hoffman]

* This better work. [Daniel Hoffman]

* Typo. [Daniel Hoffman]

* Audio playback fix on big-endian machines (tested on SPARC) [Daniel Hoffman]

* Enough with the warnings already. [Daniel Hoffman]

* Stupid typo. [Daniel Hoffman]

* Small encryption fixes. [Daniel Hoffman]

* Initial test with RSA key generation. [Daniel Hoffman]

* Converted encrypt over to DER encoding. [Daniel Hoffman]

* Simple auto-installer for Ubuntu. [Daniel Hoffman]

* Not binding to clang++ [Daniel Hoffman]

* Polishing up networking code, more well defined in and out for requests. [Daniel Hoffman]

* Little things. [Daniel Hoffman]

* Small fixes. [Daniel Hoffman]

* Basic implementation of net_ip_t. [Daniel Hoffman]

* Added basic TCP connection jargon. [Daniel Hoffman]

* Worked on net socket stats code. [Daniel Hoffman]

* Made audio test work out of the box, working out TCP outbound conn. [Daniel Hoffman]

* Fixed the makefile. [Daniel Hoffman]

* Added IRC channel. [Daniel Hoffman]

* More networking stuff. [Daniel Hoffman]

* Cleaning up here and there. [Daniel Hoffman]

* Created net_proto_socket_t with an integrated buffer for protocol networking functions. [Daniel Hoffman]

* More comments on net security, more encryption stuff. [Daniel Hoffman]

* Fixed wrong shift operators. [Daniel Hoffman]

* Fixed macro typo in IDs. [Daniel Hoffman]

* Changed PGP references to RSA, polishing up export. [Daniel Hoffman]

* Simple duplicate search, small fixes. [Daniel Hoffman]

* Fixed WAV importing, good enough for now. [Daniel Hoffman]

* WAV works, but it sounds terrible. [Daniel Hoffman]

* Crude and probably broken audio code has been added. [Daniel Hoffman]

* Allowed Mix_LoadWAV_RW to delete SDL_RWops. [Daniel Hoffman]

* Clean up RWops and more sanity. [Daniel Hoffman]

* Enabled untested WAV code. [Daniel Hoffman]

* Some initial code for tv_audio. [Daniel Hoffman]

* Audio testing. [Daniel Hoffman]

* Removed silly width and height to linked lists. [Daniel Hoffman]

* Refactored internal ID code. [Daniel Hoffman]

* Fixed flip_bit_section, little bit of audio work. [Daniel Hoffman]

* Updated README. [Daniel Hoffman]

* Split and cleaned up some of the net code. [Daniel Hoffman]

* Removed a lot of clang compiler warnings. [Daniel Hoffman]

* Added more outbound stuff, closer to working. [Daniel Hoffman]

* Worked on outbound networking, added TODO.org. [Daniel Hoffman]

* Added buffer size getters in net_socket_t. [Daniel Hoffman]

* Small net cleanups. [Daniel Hoffman]

* Removed some debugging print statements. [Daniel Hoffman]

* Initial ID transport. [Daniel Hoffman]

* Patching together export, misc. [Daniel Hoffman]

* Notes and cleaning up. [Daniel Hoffman]

* Testing sockets, misc. fixes. [Daniel Hoffman]

* Compressor, inbound post-read socket buffer, misc. [Daniel Hoffman]

* Added zlib. [Daniel Hoffman]

* Enabled pulling refresh rate. [Daniel Hoffman]

* Re-enabled copy. [Daniel Hoffman]

* Optimized frame copy. [Daniel Hoffman]

* Bothered with basic leak checking. [Daniel Hoffman]

* Small fixes and optimizations. [Daniel Hoffman]

* Fixed typo in id adder. [Daniel Hoffman]

* Cleaned up tv_render_all. [Daniel Hoffman]

* Split up tv_dev. [Daniel Hoffman]

* Destroy device data. [Daniel Hoffman]

* Comment for clarity. [Daniel Hoffman]

* Fixed mask conversions. [Daniel Hoffman]

* Created basic camera demo. [Daniel Hoffman]

* Forgot it isn't supposed to be a stream. [Daniel Hoffman]

* Mapped brightness to greyscale, doesn't stream information. [Daniel Hoffman]

* Rough V4L2 interface, no YUYV to RGB conversion. [Daniel Hoffman]

* Converted some macros to settings.cfg file. [Daniel Hoffman]

* Rough draft, but split up frame into formats. [Daniel Hoffman]

* Added template for linux device API. [Daniel Hoffman]

* Pointer drawing fixed, fixed some characters. [Daniel Hoffman]

* Built menu wrapping. [Daniel Hoffman]

* Defined resolution for tv_menu. [Daniel Hoffman]

* Ditched PGP for RSA. [Daniel Hoffman]

* Apparently the shift operators are already endian-agnostic. [Daniel Hoffman]

* Created tv_menu_entry_t. [Daniel Hoffman]

* Misc fixes while fixing ptr render, still doesn't work. [Daniel Hoffman]

* Fixed menus with pointer reference. [Daniel Hoffman]

* Dynamically size tv_frame_t in tv_menu, but broke pointer rendering. [Daniel Hoffman]

* Added custom RGBA masks, fixed W and M. [Daniel Hoffman]

* Comments to tv_menu file, fixed X. [Daniel Hoffman]

* Added border macro for tv_menu. [Daniel Hoffman]

* G and T fix. [Daniel Hoffman]

* Fixed O glyph. [Daniel Hoffman]

* Tv_menu demo. [Daniel Hoffman]

* Tv_menu template. [Daniel Hoffman]

* Removed un-needed pointer in scaling. [Daniel Hoffman]

* Never and always macros, window pos as macros, scaling windows. [Daniel Hoffman]

* Change resolution to 16:9. [Daniel Hoffman]

* Re-added different BPCs. [Daniel Hoffman]

* Fixed xor frame function. [Daniel Hoffman]

* Use references for SDL_Surface. [Daniel Hoffman]

* Defined P_SPAM_ macro for optimization, removed old spam. [Daniel Hoffman]

* Changed frame generation so it works on RPi. [Daniel Hoffman]

* Changed Makefile back. [Daniel Hoffman]

* Added temp timer for RPi. [Daniel Hoffman]

* Fixed frame demo, added signals. [Daniel Hoffman]

* Linked list test with tv_frame_t, works okay. [Daniel Hoffman]

* Fixed frame. [Daniel Hoffman]

* SDL rendering working. [Daniel Hoffman]

* More tv framework, cleanup. [Daniel Hoffman]

* Version numbers, started linked lists in IDs. [Daniel Hoffman]

* 'type' changed from string to array. [Daniel Hoffman]

* Macro optimizations for NBO. [Daniel Hoffman]

* Fixed comments. [Daniel Hoffman]

* Updated comments in types.h. [Daniel Hoffman]

* Added types and general conversion stuff. [Daniel Hoffman]

* TCP sockets test. [Daniel Hoffman]

* True_rand, misc changes, nothing too big. [Daniel Hoffman]

* TCP sockets test. [Daniel Hoffman]

* New socket skeleton, id index type, organized. [Daniel Hoffman]

* Docs for input and IR, shell for input code. [Daniel Hoffman]

* Fixed compiling. [Daniel Hoffman]

* Little work on net_proto, basic docs. [Daniel Hoffman]

* Fixed errors. [Daniel Hoffman]

* More helper functions and type cache for ID. [Daniel Hoffman]

* Split up tv files. [Daniel Hoffman]

* Fixed compiler errors, added some more core stuff. [Daniel Hoffman]

* Placeholder files & docs. [Daniel Hoffman]

* Moved to GitHub. [Daniel Hoffman]

* Initial commit. [Daniel]


