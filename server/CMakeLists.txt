SET(GRUBCFG ${PROJECT_SOURCE_DIR}/config/x86/grub.cfg)
SET(MODPREFIX "module /boot/")
SET(NEWLINE "\n\t")
SET(modules)
SET(Link_Script ${PROJECT_SOURCE_DIR}/config/${ARCHITECT}/user.ld)
SET(CMAKE_EXE_LINKER_FLAGS "-T${Link_Script}")

# SET(SERVERS device memory)
SET(SERVERS)
SET(SERVERS ${SERVERS} PARENT_SCOPE)

FOREACH(SVR ${SERVERS})
	SET(RES "${MODPREFIX}${SVR}${NEWLINE}")	
	SET(modules "${modules}${RES}")
	SET(OUTP ${ISO_DIR}/boot/${SVR})
	ADD_SUBDIRECTORY(./${SVR})	
	ADD_CUSTOM_TARGET(
		COPY${SVR} ALL
		COMMAND echo "fdjklsajflkdsjalkfjlkdsaj"
		COMMAND mkdir ${ISO_DIR}/boot/grub -p
		COMMAND cp ./${SVR}/${SVR} ${OUTP}
		COMMENT "Copy the ${SVR} to the boot directory"
		DEPENDS ${SVR}
		)
	ADD_DEPENDENCIES(ISO COPY${SVR})
ENDFOREACH()

MESSAGE(STATUS ${GRUBCFG})
CONFIGURE_FILE(${GRUBCFG}.config ${ISO_DIR}/boot/grub/grub.cfg)
