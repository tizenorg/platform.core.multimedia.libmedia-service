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

TS10=utc_minfo_finalize_func
TS11=utc_minfo_destroy_mtype_item_func
TS13=utc_minfo_get_bookmark_list_func
TS14=utc_minfo_get_cluster_cnt_func
TS15=utc_minfo_get_cluster_cover_func
TS16=utc_minfo_get_cluster_func
TS17=utc_minfo_get_cluster_list_func
TS18=utc_minfo_get_cluster_id_by_url_func
TS19=utc_minfo_get_geo_item_list_func
TS20=utc_minfo_get_item_cnt_func
TS21=utc_minfo_get_item_func
TS22=utc_minfo_get_item_list_func
TS25=utc_minfo_get_meta_info_func
TS26=utc_minfo_get_thumb_path_func
TS27=utc_minfo_init_func
TS28=utc_minfo_move_media_func
TS29=utc_minfo_mv_media_func
TS30=utc_minfo_update_cluster_name_func
TS31=utc_minfo_update_favorite_by_media_id_func
TS32=utc_minfo_update_media_favorite_func
TS33=utc_minfo_update_media_name_func
TS34=utc_minfo_set_cluster_lock_status_func
TS35=utc_minfo_get_cluster_lock_status_func
TS36=utc_minfo_bookmark_func
TS37=utc_minfo_cluster_func
TS38=utc_minfo_media_func
TS39=utc_minfo_get_all_item_list_func
TS40=utc_minfo_update_video_meta_info_int_func
TS41=utc_minfo_get_thumb_path_for_video_func
TS42=utc_minfo_get_cluster_name_by_id_func
TS43=utc_minfo_get_media_path_func
TS44=utc_minfo_tag_func
TS45=utc_minfo_get_media_list_by_tagname_func
TS46=utc_minfo_get_tag_list_by_media_id_func
TS47=utc_minfo_set_db_valid_func
TS48=utc_minfo_delete_invalid_media_records_func
TS49=utc_minfo_get_cluster_fullpath_by_id_func


LIBS = `pkg-config --libs libmedia-info`
LIBS +=-L/usr/lib/:/usr/lib/pkgconfig
LIBS +=$(TET_ROOT)/lib/tet3/tcm_s.o
LIBS +=-L$(TET_ROOT)/lib/tet3 -ltcm_s
LIBS +=-L$(TET_ROOT)/lib/tet3/ -lapi_s

INCS = -I. `pkg-config --cflags  libmedia-info`
INCS += -I$(TET_ROOT)/inc/tet3
INCS += -I/usr/include

CFLAGS = $(INCS) 
CC += $(CFLAGS)
LDFLAGS = $(LIBS)

all : $(TS10) $(TS11) $(TS12) $(TS13) $(TS14) $(TS15) $(TS16) $(TS17) $(TS18) $(TS19) $(TS20) $(TS21) $(TS22) $(TS23) $(TS24) $(TS25) $(TS26) $(TS27) $(TS28) $(TS29) $(TS30) $(TS31) $(TS32) $(TS33) $(TS34) $(TS35) $(TS36) $(TS37) $(TS38) $(TS39) $(TS40) $(TS41) $(TS42) $(TS43) $(TS44) $(TS45) $(TS46) $(TS47) $(TS48) $(TS49)

$(TS1): $(TS1).c
	$(CC) -o $(TS1) $(TS1).c $(LDFLAGS)

$(TS2): $(TS2).c
	$(CC) -o $(TS2) $(TS2).c $(LDFLAGS)
	
$(TS3): $(TS3).c
	$(CC) -o $(TS3) $(TS3).c $(LDFLAGS)

$(TS4): $(TS4).c
	$(CC) -o $(TS4) $(TS4).c $(LDFLAGS)

$(TS5): $(TS5).c
	$(CC) -o $(TS5) $(TS5).c $(LDFLAGS)
	
$(TS6): $(TS6).c
	$(CC) -o $(TS6) $(TS6).c $(LDFLAGS)

$(TS7): $(TS7).c
	$(CC) -o $(TS7) $(TS7).c $(LDFLAGS)

$(TS8): $(TS8).c
	$(CC) -o $(TS8) $(TS8).c $(LDFLAGS)

$(TS9): $(TS9).c
	$(CC) -o $(TS9) $(TS9).c $(LDFLAGS)

$(TS10): $(TS10).c
	$(CC) -o $(TS10) $(TS10).c $(LDFLAGS)

$(TS11): $(TS11).c
	$(CC) -o $(TS11) $(TS11).c $(LDFLAGS)

$(TS12): $(TS12).c
	$(CC) -o $(TS12) $(TS12).c $(LDFLAGS)

$(TS13): $(TS13).c
	$(CC) -o $(TS13) $(TS13).c $(LDFLAGS)

$(TS14): $(TS14).c
	$(CC) -o $(TS14) $(TS14).c $(LDFLAGS)

$(TS15): $(TS15).c
	$(CC) -o $(TS15) $(TS15).c $(LDFLAGS)

$(TS16): $(TS16).c
	$(CC) -o $(TS16) $(TS16).c $(LDFLAGS)

$(TS17): $(TS17).c
	$(CC) -o $(TS17) $(TS17).c $(LDFLAGS)

$(TS18): $(TS18).c
	$(CC) -o $(TS18) $(TS18).c $(LDFLAGS)

$(TS19): $(TS19).c
	$(CC) -o $(TS19) $(TS19).c $(LDFLAGS)

$(TS20): $(TS20).c
	$(CC) -o $(TS20) $(TS20).c $(LDFLAGS)

$(TS21): $(TS21).c
	$(CC) -o $(TS21) $(TS21).c $(LDFLAGS)

$(TS22): $(TS22).c
	$(CC) -o $(TS22) $(TS22).c $(LDFLAGS)

$(TS23): $(TS23).c
	$(CC) -o $(TS23) $(TS23).c $(LDFLAGS)

$(TS24): $(TS24).c
	$(CC) -o $(TS24) $(TS24).c $(LDFLAGS)

$(TS25): $(TS25).c
	$(CC) -o $(TS25) $(TS25).c $(LDFLAGS)

$(TS26): $(TS26).c
	$(CC) -o $(TS26) $(TS26).c $(LDFLAGS)

$(TS27): $(TS27).c
	$(CC) -o $(TS27) $(TS27).c $(LDFLAGS)

$(TS28): $(TS28).c
	$(CC) -o $(TS28) $(TS28).c $(LDFLAGS)

$(TS29): $(TS29).c
	$(CC) -o $(TS29) $(TS29).c $(LDFLAGS)

$(TS30): $(TS30).c
	$(CC) -o $(TS30) $(TS30).c $(LDFLAGS)

$(TS31): $(TS31).c
	$(CC) -o $(TS31) $(TS31).c $(LDFLAGS)

$(TS32): $(TS32).c
	$(CC) -o $(TS32) $(TS32).c $(LDFLAGS)

$(TS33): $(TS33).c
	$(CC) -o $(TS33) $(TS33).c $(LDFLAGS)

$(TS34): $(TS34).c
	$(CC) -o $(TS34) $(TS34).c $(LDFLAGS)

$(TS35): $(TS35).c
	$(CC) -o $(TS35) $(TS35).c $(LDFLAGS)

$(TS36): $(TS36).c
	$(CC) -o $(TS36) $(TS36).c $(LDFLAGS)

$(TS37): $(TS37).c
	$(CC) -o $(TS37) $(TS37).c $(LDFLAGS)

$(TS38): $(TS38).c
	$(CC) -o $(TS38) $(TS38).c $(LDFLAGS)

$(TS39): $(TS39).c
	$(CC) -o $(TS39) $(TS39).c $(LDFLAGS)

$(TS40): $(TS40).c
	$(CC) -o $(TS40) $(TS40).c $(LDFLAGS)

$(TS41): $(TS41).c
	$(CC) -o $(TS41) $(TS41).c $(LDFLAGS)

$(TS42): $(TS42).c
	$(CC) -o $(TS42) $(TS42).c $(LDFLAGS)

$(TS43): $(TS43).c
	$(CC) -o $(TS43) $(TS43).c $(LDFLAGS)

$(TS44): $(TS44).c
	$(CC) -o $(TS44) $(TS44).c $(LDFLAGS)

$(TS45): $(TS45).c
	$(CC) -o $(TS45) $(TS45).c $(LDFLAGS)

$(TS46): $(TS46).c
	$(CC) -o $(TS46) $(TS46).c $(LDFLAGS)

$(TS47): $(TS47).c
	$(CC) -o $(TS47) $(TS47).c $(LDFLAGS)

$(TS48): $(TS48).c
	$(CC) -o $(TS48) $(TS48).c $(LDFLAGS)

$(TS49): $(TS49).c
	$(CC) -o $(TS49) $(TS49).c $(LDFLAGS)

clean:
	rm -rf *~ *.o $(TS10) $(TS11) $(TS12) $(TS13) $(TS14) $(TS15) $(TS16) $(TS17) $(TS18) $(TS19) $(TS20) $(TS21) $(TS22) $(TS23) $(TS24) $(TS25) $(TS26) $(TS27) $(TS28) $(TS29) $(TS30) $(TS31) $(TS32) $(TS33) $(TS34) $(TS35) $(TS36) $(TS37) $(TS38) $(TS39) $(TS40) $(TS41) $(TS42) $(TS43) $(TS44) $(TS45) $(TS46) $(TS47) $(TS48) $(TS49)
	


