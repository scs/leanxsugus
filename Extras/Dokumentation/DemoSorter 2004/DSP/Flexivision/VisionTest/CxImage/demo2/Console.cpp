// Console.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "ximage.h"

CString FindExtension(const CString& name)
{
	int len = name.GetLength();
	int i;
	for (i = len-1; i >= 0; i--){
		if (name[i] == '.'){
			return name.Mid(i+1);
		}
	}
	return CString("");
}

int FindFormat(const CString& ext)
{
	if (ext == "bmp")						return CXIMAGE_FORMAT_BMP;
	else if (ext == "gif")				    return CXIMAGE_FORMAT_GIF;
	else if (ext == "ico")				    return CXIMAGE_FORMAT_ICO;
	else if (ext == "tga")				    return CXIMAGE_FORMAT_TGA;
	else if (ext == "jpg")				    return CXIMAGE_FORMAT_JPG;
    else if (ext == "tif" || ext=="tiff")   return CXIMAGE_FORMAT_TIF;
    else if (ext == "png")                  return CXIMAGE_FORMAT_PNG;
    else if (ext == "wbmp")                 return CXIMAGE_FORMAT_WBMP;
	else if (ext == "wmf" || ext =="emf")   return CXIMAGE_FORMAT_WMF;
    else if (ext == "pcx")                  return CXIMAGE_FORMAT_PCX;
    //else if (ext == "j2k" || ext =="jp2") return CXIMAGE_FORMAT_J2K;
    //else if (ext == "jbg")                return CXIMAGE_FORMAT_JBG;
	else return CXIMAGE_FORMAT_UNKNOWN;
}

int main(int argc, char* argv[])
{

    if (argc<3) {
        fprintf(stderr, "CxImage 5.00 - Console demo\n");
        fprintf(stderr, "usage: %s input-file output-file\n", argv[0]);
        return 1;
    }

	CString filein(argv[1]);
	CString extin(FindExtension(filein));
	extin.MakeLower();
	int typein = FindFormat(extin);
	if (typein == CXIMAGE_FORMAT_UNKNOWN) {
        fprintf(stderr, "unknown extension for %s\n", argv[1]);
        return 1;
	}

	CString fileout(argv[2]);
	CString extout(FindExtension(fileout));
	extout.MakeLower();
	int typeout = FindFormat(extout);
	if (typeout == CXIMAGE_FORMAT_UNKNOWN) {
        fprintf(stderr, "unknown extension for %s\n", argv[2]);
        return 1;
	}

	CxImage image;

	if (!image.Load(argv[1],typein)){
        fprintf(stderr, "%s\n", image.GetLastError());
        fprintf(stderr, "error loading %s\n", argv[1]);
        return 1;
	}

	if (!image.Save(argv[2],typeout)){
        fprintf(stderr, "%s\n", image.GetLastError());
        fprintf(stderr, "error saving %s\n", argv[2]);
        return 1;
	}

	printf("Done!\n");
	return 0;
}

