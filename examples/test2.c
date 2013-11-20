#include <vfs/vfs.h>


int main(int argc,char* argv[] )
{

	vfs_file *vf;

	uvar64 realsize ;
	char buf[VFS_MAX_FILENAME+1];

	if( VFS_TRUE != vfs_create())
		goto ERROR;

	if( VFS_TRUE != vfs_pak_add("../media/test.pak"))
		goto ERROR;

	vf = vfs_fopen("bzip2/randtable.c","rb");
	if( !vf )
		goto ERROR;

	while( !vfs_feof(vf) )
	{
		realsize = vfs_fread(buf,1,(size_t)VFS_MAX_FILENAME,vf);
		if( realsize > 0 )
		{
			buf[realsize] = 0;
			printf(buf);
		}
	}

	vfs_fclose(vf);
	vfs_destroy();
	return 0;

ERROR:
	vfs_destroy();
	return -1;

}
