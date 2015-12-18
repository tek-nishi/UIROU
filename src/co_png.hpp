﻿
#pragma once

// 
// libpngによる画像読み書き
//
// PNG_COLOR_TYPE_RGB		    RGB
// PNG_COLOR_TYPE_RGB_ALPHA	RGBA
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


#include <fstream>
#include <string>
#include <vector>
#include <zlib.h>
#include <png.h>


namespace ngs {

// TIPS:RAIIによる安全なpng_structリソース管理
class PngStruct {
	png_struct* hdl_;
	png_info* info_;

public:
	PngStruct() :
		hdl_(),
		info_()
	{
		hdl_ = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
		!hdl_ && DOUT << "Error:png_create_read_struct()" << std::endl;
		
		if (hdl_) {
			info_ = png_create_info_struct(hdl_);
			!info_ && DOUT << "Error:png_create_info_struct()" << std::endl;
		}
	}

	~PngStruct() {
		if (hdl_) png_destroy_read_struct(&hdl_, &info_, 0);
	}

	bool error() const { return !hdl_ || !info_; }
	png_struct* hdl() const { return hdl_; }
	png_info* info() const { return info_; }
};


class Png {
	enum {
    PNG_SIG_HEADER = 8,
    // PNGかどうかを判別するために読み込むデータ量

    PNG_COLOR_TYPE_NONE = -1
  };

	int type_;
	int width_;
	int height_;
	std::vector<u_char> image_;


  // png読み込み時のコールバック
  static void readFunc(png_struct* hdl, png_bytep buf, png_size_t size) {
    std::ifstream* fstr = static_cast<std::ifstream*>(png_get_io_ptr(hdl));
    fstr->read(reinterpret_cast<char*>(buf), size);
    // FIXME:reinterpret_cast使いたくない…
  }
	
public:
	Png(const std::string& file) :
		type_(PNG_COLOR_TYPE_NONE)
	{
		DOUT << "Png()" << std::endl;

    std::ifstream fstr(file, std::ios::binary);
    if (!fstr) {
      DOUT << "PNG flle open error. " << file << std::endl;
      return;
    }

    // PNGかどうかを検証
    png_byte header[PNG_SIG_HEADER];
    fstr.read(reinterpret_cast<char*>(header), PNG_SIG_HEADER);
    if (int cmp = png_sig_cmp(header, 0, PNG_SIG_HEADER)) {
			DOUT << "Error:png_sig_cmp():" << cmp << std::endl;
			return;
    }

    // png読み込み用ハンドル生成
		PngStruct png;																	// FIXME:try~catchでできそう
		if (png.error()) return;

    // 標準的なFILE I/Oを使わないので、読み込み処理を自分で用意
		png_set_read_fn(png.hdl(), static_cast<png_voidp>(&fstr), static_cast<png_rw_ptr>(readFunc));

    // 検証したぶんをスキップしつつ、各種情報を取得
    png_set_sig_bytes(png.hdl(), PNG_SIG_HEADER);
		png_read_info(png.hdl(), png.info());

		png_uint_32 width, height;
		int depth;
		png_get_IHDR(png.hdl(), png.info(), &width, &height, &depth, &type_, 0, 0, 0);
    width_ = width;
    height_ = height;

		// どんなフォーマットもRGB8かRGBA8に収めるように準備
		if (type_ == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png.hdl());
		if (png_get_valid(png.hdl(), png.info(), PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png.hdl());
		if (depth == 16) png_set_strip_16(png.hdl());
		if (depth < 8) png_set_packing(png.hdl());

    // フォーマット更新
		png_read_update_info(png.hdl(), png.info());

		// 最終的なフォーマットを決定
		size_t row = png_get_rowbytes(png.hdl(), png.info());
		type_ = ((row / width) == 3) ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGB_ALPHA;
		
		// 読込先の領域を確保し、読み込みテーブルを生成
		image_.resize(row * height);
		std::vector<png_bytep> row_pointers(height);
		for (size_t h = 0; h < height; ++h) {
			row_pointers[h] = &image_[h * row];
		}
		
		png_read_image(png.hdl(), &row_pointers[0]);
		png_read_end(png.hdl(), png.info());

		DOUT << "PNG:" << width_ << " x " << height_ << std::endl;
	}
  
	~Png() {
		DOUT << "~Png()" << std::endl;
	}

  int type() const { return type_; }
  int width() const { return width_; }
  int height() const { return height_; }
	const u_char* image() const { return &image_[0]; }
  
};

//
// png書き出し
//
class PngWriteStruct {
	png_struct* hdl_;
	png_info* info_;

public:
	PngWriteStruct() :
		hdl_(0),
		info_(0)
	{
		hdl_ = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
		!hdl_ && DOUT << "Error:png_create_write_struct()" << std::endl;
		
		if (hdl_) {
			info_ = png_create_info_struct(hdl_);
			!info_ && DOUT << "Error:png_create_info_struct()" << std::endl;
		}
	}

	~PngWriteStruct() {
		if (hdl_) png_destroy_write_struct(&hdl_, &info_);
	}

	bool error() const { return !hdl_ || !info_; }
	png_struct *hdl() const { return hdl_; }
	png_info *info() const { return info_; }

};


// RGB8で書き出し
void WritePng(const std::string& file, const u_int width, const u_int height, u_char* image) {
	// TIPS:OpenGLのフレームバッファは上下逆なので、
	//      書き出しテーブルを作るときにひっくり返す
	std::vector<png_bytep> row_pointers(height);
	for (size_t h = 0; h < height; ++h) {
		row_pointers[h] = &image[(height - 1 - h) * width * 4];
	}
	
	PngWriteStruct png;                               // FIXME:try~catchでできそう
	if (png.error()) return;

	FILE* fp = fopen(file.c_str(), "wb");
	if (!fp) {
		DOUT << "File create error:" << file << std::endl;
		return;
	}

	png_init_io(png.hdl(), fp);

	png_set_IHDR(png.hdl(), png.info(), width, height, 8,
							 PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_write_info(png.hdl(), png.info());
	png_set_filler(png.hdl(), 0, PNG_FILLER_AFTER);

	png_write_image(png.hdl(), &row_pointers[0]);
	png_write_end(png.hdl(), 0);

	fclose(fp);
}

}
