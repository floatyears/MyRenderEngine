#include "resource.h"

int Load_Bitmap_File(Bitmap_File_PTR bitmap, char *filename)
{
	FILE *file_handle;
	int	index;

	UCHAR *tmp_buffer = NULL;
	OFSTRUCT file_data;			//�ļ�������Ϣ
	int error;

	std::ifstream fin(filename, std::ios::binary);
	//���ļ�
	if ((error = fopen_s(&file_handle, filename, "rb")) != 0)// &file_data, OF_READ)) == -1)
		return 0;
	
	//_lread(file_handle, &bitmap->bitmapfileheader, sizeof(BITMAPFILEHEADER));
	fread(&bitmap->bitmapfileheader, sizeof(BITMAPFILEHEADER), 1, file_handle);

	//����Ƿ���bitmap
	if (bitmap->bitmapfileheader.bfType != BITMAP_ID)
	{
		fclose(file_handle);
		//_lclose(file_handle);
		return 0;
	}
	
	//_lread(file_handle, &bitmap->bitmapinfoheader, sizeof(BITMAPINFOHEADER));
	fread(&bitmap->bitmapinfoheader, sizeof(BITMAPINFOHEADER), 1, file_handle);

	//����е�ɫ��Ļ�����ô�ͼ���
	if (bitmap->bitmapinfoheader.biBitCount == 8)
	{
		//_lread(file_handle, &bitmap->palette, MAX_COLORS_PALETTE*sizeof(PALETTEENTRY));
		fread(&bitmap->palette, MAX_COLORS_PALETTE * sizeof(PALETTEENTRY), 1, file_handle);
		for ( index = 0; index < MAX_COLORS_PALETTE; index++)
		{
			int tmp_color = bitmap->palette[index].peRed;
			bitmap->palette[index].peRed = bitmap->palette[index].peBlue;
			bitmap->palette[index].peBlue = tmp_color;

			bitmap->palette[index].peFlags = PC_NOCOLLAPSE;
		}
	}

	//_lseek(file_handle, -(int)(bitmap->bitmapinfoheader.biSizeImage), SEEK_END);
	fseek(file_handle, -(int)(bitmap->bitmapinfoheader.biSizeImage), SEEK_END);

	if (bitmap->bitmapinfoheader.biBitCount == 8 || bitmap->bitmapinfoheader.biBitCount == 16)
	{
		if (bitmap->buffer)
			free(bitmap->buffer);

		if (!(bitmap->buffer = (UCHAR *)malloc(bitmap->bitmapinfoheader.biSizeImage)))
		{
			//_lclose(file_handle);
			fclose(file_handle);

			return 0;
		}

		//_lread(file_handle, bitmap->buffer, bitmap->bitmapinfoheader.biSizeImage);
		fread(bitmap->buffer, bitmap->bitmapinfoheader.biSizeImage,1, file_handle);
	}
	else if (bitmap->bitmapinfoheader.biBitCount == 24) //24λ��λͼ
	{
		if (!(tmp_buffer = (UCHAR *)malloc(bitmap->bitmapinfoheader.biSizeImage)))
		{
			//_lclose(file_handle);
			//fread(bitmap->buffer, bitmap->bitmapinfoheader.biSizeImage, 1, file_handle);
			fclose(file_handle);

			return 0;
		}

		//�����32λ����ɫ
		if (!(bitmap->buffer = (UCHAR *)malloc(4 * bitmap->bitmapinfoheader.biWidth * bitmap->bitmapinfoheader.biHeight)))
		{
			//_lclose(file_handle);
			//fread(bitmap->buffer, bitmap->bitmapinfoheader.biSizeImage, 1, file_handle);
			fclose(file_handle);
			free(tmp_buffer);
			return 0;
		}

		//int i = bitmap->bitmapinfoheader.biSizeImage;
		////UCHAR color;
		//char color;
		//while (i >= 0)
		//{
		//	/*fseek(file_handle, -(int)(bitmap->bitmapinfoheader.biSizeImage + i), SEEK_END);

		//	fread(&color, 1, 1, file_handle);*/

		//	fin.read(&color, sizeof(char));
		//	
		//	if (i == 10000)
		//	{
		//		i = i;
		//	}
		//	i--;
		//}

		//fin.seekg(sizeof(bitmap->bitmapfileheader) + sizeof(bitmap->bitmapinfoheader), fin.beg);
		//_lread(file_handle, tmp_buffer, bitmap->bitmapinfoheader.biSizeImage);
		
		fread(tmp_buffer, bitmap->bitmapinfoheader.biSizeImage, 1, file_handle);
		//char *color = new char[3];
		for ( index = 0; index < bitmap->bitmapinfoheader.biWidth * bitmap->bitmapinfoheader.biHeight; index++)
		{
			/*if (index == 10000)
			{
				index = index;
			}*/
			
			//bitmap�ĸ�ʽ˳����b,g,r
			((UINT *)bitmap->buffer)[index] = RGBA32BIT(tmp_buffer[index * 3 + 2], tmp_buffer[index * 3 + 1], tmp_buffer[index * 3 + 0], 255);
			
			//fin.read(color, sizeof(char) * 3);
			//((UINT *)bitmap->buffer)[index] = RGBA32BIT(color[0], color[1], color[2], 255);
		}

		bitmap->bitmapinfoheader.biBitCount = 32;

		free(tmp_buffer);
	}
	else if (bitmap->bitmapinfoheader.biBitCount == 32)
	{
		if (!(tmp_buffer = (UCHAR *)malloc(bitmap->bitmapinfoheader.biSizeImage)))
		{
			//_lclose(file_handle);
			//fread(bitmap->buffer, bitmap->bitmapinfoheader.biSizeImage, 1, file_handle);
			fclose(file_handle);

			return 0;
		}

		//�����32λ����ɫ
		if (!(bitmap->buffer = (UCHAR *)malloc(4 * bitmap->bitmapinfoheader.biWidth * bitmap->bitmapinfoheader.biHeight)))
		{
			//_lclose(file_handle);
			//fread(bitmap->buffer, bitmap->bitmapinfoheader.biSizeImage, 1, file_handle);
			fclose(file_handle);
			free(tmp_buffer);
			return 0;
		}

		//int i = bitmap->bitmapinfoheader.biSizeImage;
		//UCHAR color;
		//while (i >= 0)
		//{
		//	fseek(file_handle, -(int)(bitmap->bitmapinfoheader.biSizeImage + i), SEEK_END);

		//	fread(&color, 1, 1, file_handle);
		//	i--;
		//}

		//_lread(file_handle, tmp_buffer, bitmap->bitmapinfoheader.biSizeImage);

		//fread(tmp_buffer, bitmap->bitmapinfoheader.biSizeImage, 1, file_handle);
		fread(tmp_buffer, bitmap->bitmapinfoheader.biHeight * bitmap->bitmapinfoheader.biWidth * 4, 1, file_handle);
		
		for (index = 0; index < bitmap->bitmapinfoheader.biWidth * bitmap->bitmapinfoheader.biHeight; index++)
		{
			((UINT *)bitmap->buffer)[index] = RGBA32BIT(tmp_buffer[index * 4 + 1], tmp_buffer[index * 4 + 2], tmp_buffer[index * 4 + 3], tmp_buffer[index * 4 + 0]);
		}

		bitmap->bitmapinfoheader.biBitCount = 32;

		free(tmp_buffer);
	}
	else
	{
		return 0;
	}
}

int Create_Bitmap(Bitmap_Iamge_PTR image, int x, int y, int width, int height, int bpp)
{
	//�����ڴ�
	if (!(image->buffer = (UCHAR *)malloc(width*height*(bpp >> 3))))
	{
		return 0;
	}

	image->state = BITMAP_STATE_ALIVE;
	image->attr = 0;
	image->width = width;
	image->height = height;
	image->bpp = bpp;
	image->x = x;
	image->y = y;
	image->num_bytes = width*height*(bpp >> 3);
}

int Load_Image_Bitmap(Bitmap_Iamge_PTR image, //��Ҫ�������ݵ�ͼƬ
	Bitmap_File_PTR bitmap, //��ɨ���ͼƬ
	int cx, int cy, // �� ��λ���껹�Ǿ������� ��ɨ��ͼƬ
	int mode)
{
	//����������ɫ��32λ
	if (!image)
		return 0;

	UINT *sourece_ptr,
		*dest_ptr;

	//�����ȡ���ͣ���ÿ�����ӻ��Ǿ�������ֵ
	if (mode == BITMAP_EXTRACT_MODE_CELL)
	{
		cx = cx * (image->width + 1) + 1;
		cy = cy*(image->height + 1) + 1;
	}

	sourece_ptr = (UINT *)bitmap->buffer + cy*bitmap->bitmapinfoheader.biWidth + cx;

	dest_ptr = (UINT *)image->buffer;

	if (bitmap->bitmapinfoheader.biBitCount == 32)
	{

	}
	else if (bitmap->bitmapinfoheader.biBitCount == 24)
	{

	}

	for (int index_y = 0; index_y < image->height; index_y++)
	{
		memcpy(dest_ptr, sourece_ptr, image->width * 4);

		dest_ptr += image->width;
		sourece_ptr += bitmap->bitmapinfoheader.biWidth;
	}

	image->attr |= BITMAP_ATTR_LOADED;

	return 1;
}

int Unload_Bitmap_File(Bitmap_File_PTR bitmap)
{
	if (bitmap->buffer)
	{
		free(bitmap->buffer);
		bitmap->buffer = NULL;
	}

	return 1;
}