#ifndef __BogusFS_H__
#define __BogusFS_H__

#include <Arduino.h>
#include <FS.h>

class BogusFile : public  FileImpl {
public:
	BogusFile(const char *filename, uint8_t mode) {
		strlcpy(_filename, filename, sizeof(_filename));
		_fOpen = true;};
	~BogusFile() {close(); }
	virtual size_t read(void *buf, size_t nbyte) { return 0; }
	virtual size_t write(const void *buf, size_t size) {
		// lets walk through the data and see if we have sequence numbers
		uint8_t *pb = (uint8_t*)buf;
		bool first_packet = (_file_size == 0);
		if (first_packet) _simple_send_object = (size == 500);
		
		if (_simple_send_object) {
			int32_t packet_number = 0;
			for (uint16_t i = 0; pb[i] >= '0' && pb[i] <= '9'; i++) packet_number = packet_number * 10 + pb[i] - '0';
			if (packet_number != (_last_packet_number + 1)) {
				if (_error_count < 10) 	Serial.printf("BF Sequence error %u %u\n", packet_number, _last_packet_number);
				_error_count++;
			}
			_last_packet_number = packet_number;
		} else {
			while (_offset < size) {
				int32_t packet_number = 0;
				for (uint16_t i = _offset; pb[i] >= '0' && pb[i] <= '9'; i++) packet_number = packet_number * 10 + pb[i] - '0';
				if (packet_number != (_last_packet_number + 1)) {
					if (_error_count < 10) 	Serial.printf("BF Sequence error %u %u\n", packet_number, _last_packet_number);
					_error_count++;
				}
				_last_packet_number = packet_number;
				_offset += first_packet? 500 : 512;
				first_packet = false;
			}
			_offset &= 0x1ff; // probably ok...
		}
		_file_size += size;
		//delayMicroseconds(500);
		return size;
	}
	virtual int available() { return 0;}
	virtual int peek() {return -1;}
	virtual void flush() {}
	virtual bool truncate(uint64_t size=0) {return true;};
	virtual bool seek(uint64_t pos, int mode) {return false;}
	virtual uint64_t position()  { return 0;}
	virtual uint64_t size()  { return _file_size;}
	virtual void close() {_fOpen = false;};
	virtual bool isOpen() {return _fOpen;}
	virtual const char* name() {return _filename;}
	virtual bool isDirectory() { return (strcmp(_filename, "/") == 0);}
	virtual File openNextFile(uint8_t mode=0) {return File();}
	virtual void rewindDirectory(void) {};
	char _filename[256];
	uint32_t _file_size = 0;
	uint32_t _offset = 0;
	int32_t _last_packet_number = -1;
	uint32_t _error_count = 0;
	bool _fOpen;
	bool _simple_send_object = false;
};


class BogusFS : public  FS
{
public:
	BogusFS() {};
	virtual File open(const char *filename, uint8_t mode = FILE_READ) {
		Serial.printf("BogusFS::open(%s, %x)\n", filename, mode);
		return File(new BogusFile(filename, mode));
	}
	virtual bool exists(const char *filepath) {Serial.printf("BogusFS::exists(%s)\n", filepath); return true;}
	virtual bool mkdir(const char *filepath)  {Serial.printf("BogusFS::mkdir(%s)\n", filepath); return true;}
	virtual bool rename(const char *oldfilepath, const char *newfilepath) {Serial.printf("BogusFS::rename(%s, %s)\n", oldfilepath, newfilepath); return true;}
	virtual bool remove(const char *filepath)  {Serial.printf("BogusFS::remove(%s)\n", filepath); return true;}
	virtual bool rmdir(const char *filepath)  { Serial.printf("BogusFS::rmdir(%s)\n", filepath); return true;}
	virtual uint64_t usedSize()  { return 16000000000ul;} 
	virtual uint64_t totalSize() { return 32000000000ul;}
};

#endif // __BogusFS_H__
