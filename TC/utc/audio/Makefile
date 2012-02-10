ifeq ($(ARCH),target)
	PKG_CONFIG_PATH=/usr/lib/pkgconfig
	export PKG_CONFIG_PATH
	CC=arm-linux-gcc -Wall
	CXX=arm-linux-g++ -Wall
else
	PKG_CONFIG_PATH=/usr/lib/pkgconfig
	export PKG_CONFIG_PATH
	CC=gcc -Wall
    CXX=gcc -Wall
endif

TS1=utc_audio_svc_add_item_to_playlist_func
TS2=utc_audio_svc_add_playlist_func
TS3=utc_audio_svc_check_duplicate_insertion_in_playlist_func
TS5=utc_audio_svc_get_playlist_func
TS7=utc_audio_svc_count_group_item_func
TS8=utc_audio_svc_count_list_item_func
TS9=utc_audio_svc_count_playlist_by_name_func
TS10=utc_audio_svc_count_playlist_func
TS11=utc_audio_svc_create_table_func
TS12=utc_audio_svc_delete_all_func
TS15=utc_audio_svc_delete_playlist_func
TS17=utc_audio_svc_get_group_item_func
TS18=utc_audio_svc_get_item_by_audio_id_func
TS19=utc_audio_svc_get_item_by_path_func
TS20=utc_audio_svc_get_list_item_func
TS21=utc_audio_svc_get_playlist_name_by_playlist_id_func
TS22=utc_audio_svc_get_unique_playlist_name_func
TS23=utc_audio_svc_insert_item_func
TS25=utc_audio_svc_item_free_func
TS26=utc_audio_svc_item_get_val_func
TS27=utc_audio_svc_item_new_func
TS29=utc_audio_svc_list_item_free_func
TS30=utc_audio_svc_list_item_get_func
TS31=utc_audio_svc_list_item_get_val_func
TS32=utc_audio_svc_list_item_new_func
TS34=utc_audio_svc_move_item_func
TS36=utc_audio_svc_playlist_free_func
TS37=utc_audio_svc_playlist_get_item_func
TS38=utc_audio_svc_playlist_get_val_func
TS39=utc_audio_svc_playlist_new_func
TS40=utc_audio_svc_playlist_set_val_func
TS41=utc_audio_svc_refresh_metadata_func
TS42=utc_audio_svc_remove_item_from_playlist_by_audio_id_func
TS43=utc_audio_svc_set_db_valid_func
TS44=utc_audio_svc_set_item_valid_func
TS48=utc_audio_svc_update_playlist_name_func
TS50=utc_audio_svc_get_path_by_audio_id_func
TS51=utc_audio_svc_delete_item_by_path_func
TS55=utc_audio_svc_delete_invalid_items_func
TS56=utc_audio_svc_check_item_exist_func
TS57=utc_audio_svc_get_playlist_id_by_playlist_name_func

LIBS = `pkg-config --libs libmedia-service`
LIBS +=$(TET_ROOT)/lib/tet3/tcm_s.o
LIBS +=-L$(TET_ROOT)/lib/tet3/ -ltcm_s
LIBS +=-lapi_s
LIBS +=utc_audio_svc_common.o

INCS = -I. `pkg-config --cflags  libmedia-service`
INCS += -I$(TET_ROOT)/inc/tet3
INCS += -I/usr/include

CFLAGS = $(INCS)
CC += $(CFLAGS)
LDFLAGS = $(LIBS)

TCLIST = $(TS1) $(TS2) $(TS3) $(TS4) $(TS5) $(TS7) $(TS8) $(TS9) $(TS10) $(TS11) $(TS12) $(TS15) $(TS17) $(TS18) $(TS19) $(TS20) $(TS21) $(TS22) $(TS23) $(TS25) $(TS26) $(TS27) $(TS29) $(TS30) $(TS31) $(TS32) $(TS34) $(TS35) $(TS36) $(TS37) $(TS38) $(TS39) $(TS40) $(TS41) $(TS42) $(TS43) $(TS44) $(TS48) $(TS50) $(TS51) $(TS55) $(TS56) $(TS57)

#utc_audio_svc_common :
#	$(CC) -c utc_audio_svc_common.c

all :
	$(CC) -c utc_audio_svc_common.c
	for testcase in $(TCLIST) ; do \
		$(CC) -o $$testcase $(UTIL_SRC) $$testcase.c  $(LDFLAGS) ; \
	done

utc_audio_svc_common :
	$(CC) -c utc_audio_svc_common.c

$(TS1): $(TS1).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS1) $(TS1).c $(LDFLAGS)

$(TS2): $(TS2).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS2) $(TS2).c $(LDFLAGS)

$(TS3): $(TS3).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS3) $(TS3).c $(LDFLAGS)

$(TS4): $(TS4).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS4) $(TS4).c $(LDFLAGS)

$(TS5): $(TS5).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS5) $(TS5).c $(LDFLAGS)

$(TS7): $(TS7).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS7) $(TS7).c $(LDFLAGS)

$(TS8): $(TS8).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS8) $(TS8).c $(LDFLAGS)

$(TS9): $(TS9).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS9) $(TS9).c $(LDFLAGS)

$(TS10): $(TS10).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS10) $(TS10).c $(LDFLAGS)

$(TS11): $(TS11).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS11) $(TS11).c $(LDFLAGS)

$(TS12): $(TS12).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS12) $(TS12).c $(LDFLAGS)

$(TS15): $(TS15).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS15) $(TS15).c $(LDFLAGS)

$(TS17): $(TS17).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS17) $(TS17).c $(LDFLAGS)

$(TS18): $(TS18).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS18) $(TS18).c $(LDFLAGS)

$(TS19): $(TS19).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS19) $(TS19).c $(LDFLAGS)

$(TS20): $(TS20).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS20) $(TS20).c $(LDFLAGS)

$(TS21): $(TS21).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS21) $(TS21).c $(LDFLAGS)

$(TS22): $(TS22).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS22) $(TS22).c $(LDFLAGS)

$(TS23): $(TS23).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS23) $(TS23).c $(LDFLAGS)

$(TS25): $(TS25).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS25) $(TS25).c $(LDFLAGS)

$(TS26): $(TS26).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS26) $(TS26).c $(LDFLAGS)

$(TS27): $(TS27).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS27) $(TS27).c $(LDFLAGS)

$(TS29): $(TS29).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS29) $(TS29).c $(LDFLAGS)

$(TS30): $(TS30).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS30) $(TS30).c $(LDFLAGS)

$(TS31): $(TS31).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS31) $(TS31).c $(LDFLAGS)

$(TS32): $(TS32).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS32) $(TS32).c $(LDFLAGS)

$(TS34): $(TS34).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS34) $(TS34).c $(LDFLAGS)

$(TS35): $(TS35).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS35) $(TS35).c $(LDFLAGS)

$(TS36): $(TS36).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS36) $(TS36).c $(LDFLAGS)

$(TS37): $(TS37).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS37) $(TS37).c $(LDFLAGS)

$(TS38): $(TS38).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS38) $(TS38).c $(LDFLAGS)

$(TS39): $(TS39).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS39) $(TS39).c $(LDFLAGS)

$(TS40): $(TS40).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS40) $(TS40).c $(LDFLAGS)

$(TS41): $(TS41).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS41) $(TS41).c $(LDFLAGS)

$(TS42): $(TS42).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS42) $(TS42).c $(LDFLAGS)

$(TS43): $(TS43).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS43) $(TS43).c $(LDFLAGS)

$(TS44): $(TS44).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS44) $(TS44).c $(LDFLAGS)

$(TS48): $(TS48).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS48) $(TS48).c $(LDFLAGS)

$(TS50): $(TS50).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS50) $(TS50).c $(LDFLAGS)

$(TS51): $(TS51).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS51) $(TS51).c $(LDFLAGS)

$(TS55): $(TS55).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS55) $(TS55).c $(LDFLAGS)

$(TS56): $(TS56).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS56) $(TS56).c $(LDFLAGS)

$(TS57): $(TS57).c
		$(CC) -c utc_audio_svc_common.c
	        $(CC) -o $(TS57) $(TS57).c $(LDFLAGS)

clean :
	rm -rf *~ *.o $(TCLIST)



clean :
	rm -rf *~ *.o $(TCLIST)


clean :
	rm -rf *~ *.o $(TCLIST)


