# -----------------------------------------------
# File that provides "make uninstall" target
#  We use the file 'install_manifest.txt'
# -----------------------------------------------
IF(NOT EXISTS "/home/npcdna/slam_systems/SRLCD/DBow3/build/install_manifest.txt")
  MESSAGE(FATAL_ERROR "Cannot find install manifest: \"/home/npcdna/slam_systems/SRLCD/DBow3/build/install_manifest.txt\"")
ENDIF(NOT EXISTS "/home/npcdna/slam_systems/SRLCD/DBow3/build/install_manifest.txt")

FILE(READ "/home/npcdna/slam_systems/SRLCD/DBow3/build/install_manifest.txt" files)
STRING(REGEX REPLACE "\n" ";" files "${files}")
FOREACH(file ${files})
  MESSAGE(STATUS "Uninstalling \"$ENV{DESTDIR}${file}\"")
#  IF(EXISTS "$ENV{DESTDIR}${file}")
#    EXEC_PROGRAM(
#      "/usr/cmake-3.15/cmake-3.15.6-Linux-x86_64/bin/cmake" ARGS "-E remove \"$ENV{DESTDIR}${file}\""
#      OUTPUT_VARIABLE rm_out
#      RETURN_VALUE rm_retval
#      )
	EXECUTE_PROCESS(COMMAND rm $ENV{DESTDIR}${file})
#    IF(NOT "${rm_retval}" STREQUAL 0)
#      MESSAGE(FATAL_ERROR "Problem when removing \"$ENV{DESTDIR}${file}\"")
#    ENDIF(NOT "${rm_retval}" STREQUAL 0)
#  ELSE(EXISTS "$ENV{DESTDIR}${file}")
#    MESSAGE(STATUS "File \"$ENV{DESTDIR}${file}\" does not exist.")
#  ENDIF(EXISTS "$ENV{DESTDIR}${file}")
ENDFOREACH(file)


