set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_sources_from_current_dir(utils
  utils/agent_mutex_name.c
  utils/agent_named_pipe_name.c
  utils/arm_arch_queries.c
  utils/aux_match_opt.c
  utils/centre_window.c
  utils/cryptoapi.c
  utils/defaults.c
  utils/dll_hijacking_protection.c
  utils/dputs.c
  utils/escape_registry_key.c
  utils/filename.c
  utils/fontspec.c
  utils/getdlgitemtext_alloc.c
  utils/get_system_dir.c
  utils/get_username.c
  utils/interprocess_mutex.c
  utils/is_console_handle.c
  utils/load_system32_dll.c
  utils/ltime.c
  utils/makedlgitemborderless.c
  utils/make_spr_sw_abort_winerror.c
  utils/message_box.c
  utils/minefield.c
  utils/open_for_write_would_lose_data.c
  utils/pgp_fingerprints_msgbox.c
  utils/platform_get_x_display.c
  utils/registry.c
  utils/request_file.c
  utils/screenshot.c
  utils/security.c
  utils/shinydialogbox.c
  utils/split_into_argv.c
  utils/version.c
  utils/win_strerror.c
  unicode.c)
if(NOT HAVE_STRTOUMAX)
  add_sources_from_current_dir(utils utils/strtoumax.c)
endif()
add_sources_from_current_dir(eventloop
  cliloop.c handle-wait.c)
add_sources_from_current_dir(console
  select-cli.c nohelp.c console.c)
add_sources_from_current_dir(settings
  storage.c)
add_sources_from_current_dir(network
  network.c handle-socket.c named-pipe-client.c named-pipe-server.c local-proxy.c x11.c)
add_sources_from_current_dir(sshcommon
  noise.c)
add_sources_from_current_dir(sshclient
  agent-client.c gss.c sharing.c)
add_sources_from_current_dir(otherbackends
  serial.c)

# This object awkwardly needs to live in the network library as well
# as the eventloop library, in case it didn't get pulled in from the
# latter before handle-socket.c needed it.
add_library(handle-io OBJECT
  handle-io.c)
target_sources(eventloop PRIVATE $<TARGET_OBJECTS:handle-io>)
target_sources(network PRIVATE $<TARGET_OBJECTS:handle-io>)
target_include_directories(handle-io PRIVATE
    ${putty_SOURCE_DIR}
    ${GENERATED_SOURCES_DIR})
VISIT_TOOLS_ADD_FOLDER(qtssh handle-io)

add_sources_from_current_dir(qtssh no-jump-list.c nohelp.c plink.rc)
#add_dependencies(qtssh generated_licence_h)

