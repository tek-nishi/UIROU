
#pragma once

//
// zlibによる圧縮/伸張
//

// リンクするライブラリの定義(Windows)
#if defined (_MSC_VER)
#ifdef _DEBUG
#pragma comment (lib, "zlibd.lib")
#pragma comment (lib, "libpngd.lib")
#else
#pragma comment (lib, "zlib.lib")
#pragma comment (lib, "libpng.lib")
#endif
#endif


#include <vector>
#include <fstream>
#include <zlib.h>


namespace ngs {
namespace zlib {

enum {
	OUTBUFSIZ = 1024 * 16,
};

// 伸張
void encode(std::vector<char>& output, const std::vector<char>& input) {
	z_stream z;
	z.zalloc = Z_NULL;
	z.zfree	 = Z_NULL;
	z.opaque = Z_NULL;
	deflateInit(&z, Z_DEFAULT_COMPRESSION);

  // TIPS:安全にキャストする為に、一旦 void* にキャストする
	const void* in = &input[0];
	z.next_in  = const_cast<Bytef*>(static_cast<const Bytef*>(in));
	z.avail_in = static_cast<u_int>(input.size());

	char outbuf[OUTBUFSIZ];
	void* out = outbuf;
	z.next_out  = static_cast<Bytef*>(out);
	z.avail_out = OUTBUFSIZ;
	while (1) {
		int status = deflate(&z, Z_FINISH);
		if (status == Z_STREAM_END) break;
		if (z.avail_out == 0) {
			output.insert(output.end(), &outbuf[0], &outbuf[OUTBUFSIZ]);

      // TIPS:一旦 void* にすると安全にキャストできる
			void* z_out = outbuf;
			z.next_out  = static_cast<Bytef*>(z_out);
			z.avail_out = OUTBUFSIZ;
		}
	}

	int count = OUTBUFSIZ - z.avail_out;
	if (count != 0) {
			output.insert(output.end(), &outbuf[0], &outbuf[count]);
	}
	deflateEnd(&z);
}

// 圧縮
void decode(std::vector<char>& output, const std::vector<char>& input) {
	z_stream z;
	z.zalloc = Z_NULL;
	z.zfree	 = Z_NULL;
	z.opaque = Z_NULL;
	inflateInit(&z);
	
	const void* in = &input[0];
	z.next_in  = const_cast<Bytef*>(static_cast<const Bytef *>(in));
	z.avail_in = static_cast<u_int>(input.size());

	char outbuf[OUTBUFSIZ];
	void* out = outbuf;
	z.next_out  = static_cast<Bytef*>(out);
	z.avail_out = OUTBUFSIZ;
	while (1) {
		int status = inflate(&z, Z_NO_FLUSH);
		if (status == Z_STREAM_END) break;
		if (z.avail_out == 0) {
			output.insert(output.end(), &outbuf[0], &outbuf[OUTBUFSIZ]);

      // TIPS:一旦 void* にすると安全にキャストできる
			void* z_out = outbuf;
			z.next_out  = static_cast<Bytef*>(z_out);
			z.avail_out = OUTBUFSIZ;
		}
	}

	int count = OUTBUFSIZ - z.avail_out;
	if (count != 0) {
			output.insert(output.end(), &outbuf[0], &outbuf[count]);
	}
	inflateEnd(&z);
}


// 書き出し
void write(const std::string& file, const std::vector<char>& input) {
	std::vector<char> output;
	encode(output, input);

	std::ofstream fstr(file, std::ios::binary);
	if (fstr) {
		// SOURCE:http://blogs.wankuma.com/episteme/archive/2009/01/09/166002.aspx
		std::copy(output.begin(), output.end(), std::ostreambuf_iterator<char>(fstr));
	}
}

// 読み込み
void read(const std::string& file, std::vector<char>& output)
{
	std::ifstream fstr(file, std::ios::binary);
	if (fstr) {
    // ファイルサイズ取得
		size_t size = static_cast<size_t>(fstr.seekg(0, std::ios::end).tellg());

    // シーク位置を先頭に戻して読み込む
		std::vector<char> input(size);
		fstr.seekg(0, std::ios::beg).read(&input[0], size);
    
		decode(output, input);
	}
}

}
}
