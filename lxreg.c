/* super simple linux registry */
/* Stefan Koch 2012-08-02      */
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

#define TYPE_UNKNOWN  0
#define TYPE_NUM32   'n'
#define TYPE_STRING  's'

#define OP_GET 0
#define OP_SET 1

#define LXREG_VER_MAX_SIZE 12

int __lxreg_filesize(fd)
{
	struct stat st;
	
	if (0 != fstat(fd, &st)) {
		perror("lxreg: stat:");
		return -1;
	}
	return st.st_size;
}

int __lxreg_validate_type(int fd, char type)
{
	int fsize, exact, intended_size, is_type;

	fsize = __lxreg_filesize(fd) - 1; /* w/o type */
	exact = 0;

	/* get entry type if file is at least 1 byte */
	if (fsize < 0) {
		return -1;
	}
	if (1 != read(fd, &is_type, 1)) {
		perror("lxreg: read of type\n");
		return -1;
	}

	/* check if type matches desired type */
	if (is_type != type) {
		printf("lxreg: type mismatch\n");
		return -1;
	}

	/* do a santity check that filesize is reasonable */
	switch (is_type) {
		case TYPE_NUM32:
			/* at least 4 byte, rest ignored (e.g. CR) */
			intended_size = 4;
			exact         = 0;

		case TYPE_STRING:
			/* can be empty */
			intended_size = 1;
			exact         = 0;
			break;
		default:
			printf("lxreg: unknown type\n");
			return -1;
	}

	/* now test */
	if (1 == exact) {
		if (fsize != intended_size) {
			printf("lxreg: file size fails exact match\n");
			return -1;
		}
	}
	if (fsize < intended_size) {
			printf("lxreg: file size fails minimum size\n");
			return -1;
	}

	return fsize;
}

int __lxreg_open(int operation, char *location, char type, int *ret_size)
{
	int fd, len;

	/* try to open */
	fd = open(location, O_RDWR | O_CREAT, 0666);
	if (1 > fd) {
		perror("lxreg");
		return -1;
	}

	if (OP_GET == operation) {
		/* check if we have the right type and get file lentgh */
		len =  __lxreg_validate_type(fd, type);
		if (-1 == len) {
			return -1;
		}
		*ret_size = len;
	}
	
	return fd;
}

int __lxreg_read_string(int fd, char *str, int size)
{
	int read_bytes = 0, rc;

	memset((void*)str, 0, size);

	while (read_bytes < size) {
		rc = read(fd, str + read_bytes, size - read_bytes);
		if (0 > rc) {
			perror("lxreg: read string\n");
			return -1;
		}
		read_bytes += rc;
	}
}

int __lxreg_write_string(int fd, char type, char *str, int size)
{
	int written_bytes = 0, rc;
	char char_type;

	if (1 != write(fd, &type, 1)) {
		perror("lxreg: write of type");
		return -1;
	}

	while (written_bytes < size) {
		rc = write(fd, str + written_bytes, size - written_bytes);
		if (0 > rc) {
			perror("lxreg: write string\n");
			return -1;
		}
		written_bytes += rc;
	}
}

int lxreg_connect(const char *location)
{
	const char *id_file = "/lxreg.id";
	char *filebuf;
	int   fd, size;
	char  type, version[LXREG_VER_MAX_SIZE + 1];
	int   location_len, idfile_len, filebuf_len;

	memset((char*)version, 0, LXREG_VER_MAX_SIZE + 1);

	/* get length of location and file, check size */
	location_len = strlen(location);
	if ((0 == location_len) || (256 < location_len)) {
		printf("lxreg: location string too long or 0\n");
		return -1;
	}
	idfile_len = strlen(id_file);

	/* allocate memory to specify file name */
	filebuf_len = location_len + idfile_len;
	filebuf = (char*) malloc(filebuf_len + 1);
	if (NULL == filebuf) {
		printf("lxreg: out of memory\n");
		return -1;
	}

	/* construct file name */
	memset((void*)filebuf, 0, filebuf_len + 1);
	strncpy(filebuf,                location, location_len);
	strncpy(filebuf + location_len, id_file,   idfile_len);

	fd = __lxreg_open(OP_GET, (char*)filebuf, TYPE_STRING, &size);
	free(filebuf);

	if (3 > fd) {
		printf("lxreg: could not open location\n");
		return -1;
	}
	if (size > LXREG_VER_MAX_SIZE) {
		printf("lxreg: id file with wrong version size \n");
		return -1;
	}

	if (0 > __lxreg_read_string(fd, version, size)) {
		return -1;
	}
	
	printf("lxreg initialized, version = %s\n", version);
	return 0;
}

int lxreg_get_int32(char *location, int *value)
{
	int fd, size;
	fd = __lxreg_open(OP_GET, location, TYPE_NUM32, &size);
	if (0 > __lxreg_read_string(fd, (char*)value, 4)) {
		printf("lxreg: read int32 failed\n");
		return -1;
	}
	return 0;
}

int lxreg_set_int32(char *location, int value)
{
	int fd, size;
	fd = __lxreg_open(OP_SET, location, TYPE_NUM32, &size);
	if (0 > __lxreg_write_string(fd, TYPE_NUM32, (char*)&value, 4)) {
		printf("lxreg: write int32 failed\n");
		return -1;
	}
	return 0;
}
