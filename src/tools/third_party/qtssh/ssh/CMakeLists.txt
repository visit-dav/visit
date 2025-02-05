add_library(sshcommon OBJECT
  bpp1.c
  bpp2.c
  bpp-bare.c
  ca-config.c
  censor1.c
  censor2.c
  common.c
  connection1.c
  connection2.c
  crc-attack-detector.c
  gssc.c
  login1.c
  pgssapi.c
  portfwd.c
  ../sshpubk.c
  ../sshrand.c
  transient-hostkey-cache.c
  transport2.c
  verstring.c
  x11fwd.c
  zlib.c)

target_include_directories(sshcommon PRIVATE
    ${putty_SOURCE_DIR}
    ${putty_SOURCE_DIR}/${platform}
    ${GENERATED_SOURCES_DIR})

VISIT_TOOLS_ADD_FOLDER(qtssh sshcommon)

add_library(sshclient STATIC
  agentf.c
  connection1-client.c
  connection2-client.c
  kex2-client.c
  mainchan.c
  sharing.c
  ssh.c
  userauth2-client.c
  $<TARGET_OBJECTS:sshcommon>
  $<TARGET_OBJECTS:all-backends>
  $<TARGET_OBJECTS:logging>)

target_include_directories(sshclient PRIVATE
    ${putty_SOURCE_DIR}
    ${putty_SOURCE_DIR}/${platform}
    ${GENERATED_SOURCES_DIR})
VISIT_TOOLS_ADD_FOLDER(qtssh sshclient)

