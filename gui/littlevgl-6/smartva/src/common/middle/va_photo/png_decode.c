#include "png.h"
#include "jmorecfg.h"

int png_decode(char *filename,void *output_buf,int *width,int *height,int *comp)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_uint_32 img_width, img_height;
	int bit_depth, color_type, interlace_type,channels;
	FILE *fp;
	int number_passes = 1;
	unsigned char *p_buf = (unsigned char *)output_buf;

	if((fp = fopen(filename, "rb")) == NULL)
		return -1;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL, NULL, NULL);
	if (png_ptr == NULL)
	{
		fclose(fp);
		return -1;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		fclose(fp);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return -1;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		printf("png decode err!\n");
		png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
		fclose(fp);
		return -1;
	}

	png_init_io(png_ptr, fp);


	png_read_info(png_ptr, info_ptr);

	png_get_IHDR(png_ptr, info_ptr, &img_width, &img_height, &bit_depth, &color_type,
				&interlace_type, int_p_NULL, int_p_NULL);
	channels = png_get_channels(png_ptr,info_ptr);

	printf("width  = %ld\n",img_width);
	printf("height = %ld\n",img_height);
	printf("depth  = %d\n",bit_depth);
	printf("channels = %d\n",channels);
	printf("color_type = %d\n",color_type);
	printf("interlace_type = %d\n",interlace_type);

	png_set_strip_16(png_ptr);//strip bit_depth 16->8


	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png_ptr);

	// if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))//转换RGB到RGBA
	//	png_set_tRNS_to_alpha(png_ptr);


   png_color_16 my_background, *image_background;

   if (png_get_bKGD(png_ptr, info_ptr, &image_background))
      png_set_background(png_ptr, image_background,
                         PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
   else
      png_set_background(png_ptr, &my_background,
                         PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);

   /* Flip the RGB pixels to BGR (or RGBA to BGRA) */
//    if (color_type & PNG_COLOR_MASK_COLOR)
//       png_set_bgr(png_ptr);

	// if (interlacing)
	{
		number_passes = png_set_interlace_handling(png_ptr);
	}
	printf("number_passes = %d\n",number_passes);

	png_read_update_info(png_ptr, info_ptr);

	int rowbytes = png_get_rowbytes(png_ptr,info_ptr);
	int pass,y;
	for (pass = 0; pass < number_passes; pass++)
	{
		for (y = 0; y < img_height; y++)
		{
			png_read_rows(png_ptr, &p_buf, png_bytepp_NULL, 1);
			p_buf += rowbytes;
		}

    //   for (y = 0; y < height; y += number_of_rows)
    //   {
    //      png_read_rows(png_ptr, &row_pointers[y], png_bytepp_NULL,number_of_rows);
    //     //  png_read_rows(png_ptr, png_bytepp_NULL, &row_pointers[y],number_of_rows);
    //   }
   }

   png_read_end(png_ptr, info_ptr);


   png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);


   fclose(fp);

   *width = img_width;
   *height = img_height;
   *comp = channels;
   return 0;
}
