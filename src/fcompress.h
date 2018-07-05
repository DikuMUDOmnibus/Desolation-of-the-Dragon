char gz_fread_letter( gzFile gzfp );
int gz_fread_number( gzFile gzfp );
char *gz_fread_string( gzFile gzfp );
char *gz_fread_string_nohash( gzFile gzfp );
void gz_fread_to_eol( gzFile gzfp );
char *gz_fread_line( gzFile gzfp, char *line );
char *gz_fread_word( gzFile gzfp, char *word );

