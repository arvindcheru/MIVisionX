/*
 * Copyright (c) 2012-2014 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */

#include "test_engine/test.h"
#include <VX/vx.h>
#include <VX/vxu.h>
#include "shared_functions.h"


TESTCASE(Sobel3x3, CT_VXContext, ct_setup_vx_context, 0)


TEST(Sobel3x3, testNodeCreation)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_x_image = 0, dst_y_image;
    vx_graph graph = 0;
    vx_node node = 0;

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(dst_x_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(dst_y_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = vxSobel3x3Node(graph, src_image, dst_x_image, dst_y_image), VX_TYPE_NODE);

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);
    vxReleaseImage(&dst_x_image);
    vxReleaseImage(&dst_y_image);
    vxReleaseImage(&src_image);

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(dst_x_image == 0);
    ASSERT(dst_y_image == 0);
    ASSERT(src_image == 0);
}


// Generate input to cover these requirements:
// There should be a image with randomly generated pixel intensities.
static CT_Image sobel3x3_generate_random(const char* fileName, int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    return image;
}

static CT_Image sobel3x3_read_image(const char* fileName, int width, int height)
{
    CT_Image image = NULL;
    ASSERT_(return 0, width == 0 && height == 0);
    image = ct_read_image(fileName, 1);
    ASSERT_(return 0, image);
    ASSERT_(return 0, image->format == VX_DF_IMAGE_U8);
    return image;
}

static int16_t sobel_x_get(int32_t *values)
{
    int32_t res = (-values[0])     + (values[2]) +
                  (-values[3] * 2) + (values[5] * 2) +
                  (-values[6])     + (values[8]);
    return (int16_t)res;
}

static int16_t sobel_y_get(int32_t *values)
{
    int32_t res = (-values[0])     + (values[6]) +
                  (-values[1] * 2) + (values[7] * 2) +
                  (-values[2])     + (values[8]);
    return (int16_t)res;
}

static void sobel3x3_calculate(CT_Image src, uint32_t x, uint32_t y, int16_t *sobel_x, int16_t *sobel_y)
{
    int32_t values[9] = {
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y + 1)
    };
    *sobel_x = sobel_x_get(values);
    *sobel_y = sobel_y_get(values);
}

static void sobel3x3_calculate_replicate(CT_Image src, uint32_t x_, uint32_t y_, int16_t *sobel_x, int16_t *sobel_y)
{
    int32_t x = (int)x_;
    int32_t y = (int)y_;
    int32_t values[9] = {
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y + 1)
    };
    *sobel_x = sobel_x_get(values);
    *sobel_y = sobel_y_get(values);
}

static void sobel3x3_calculate_constant(CT_Image src, uint32_t x_, uint32_t y_, vx_uint32 constant_value, int16_t *sobel_x, int16_t *sobel_y)
{
    int32_t x = (int)x_;
    int32_t y = (int)y_;
    int32_t values[9] = {
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y + 1, constant_value)
    };
    *sobel_x = sobel_x_get(values);
    *sobel_y = sobel_y_get(values);
}


void sobel3x3_create_reference_image(CT_Image src, vx_border_mode_t border, CT_Image *p_dst_x, CT_Image *p_dst_y)
{
    CT_Image dst_x = NULL, dst_y = NULL;

    CT_ASSERT(src->format == VX_DF_IMAGE_U8);

    dst_x = ct_allocate_image(src->width, src->height, VX_DF_IMAGE_S16);
    dst_y = ct_allocate_image(src->width, src->height, VX_DF_IMAGE_S16);

    if (border.mode == VX_BORDER_MODE_UNDEFINED)
    {
        CT_FILL_IMAGE_16S(return, dst_x,
                if (x >= 1 && y >= 1 && x < src->width - 1 && y < src->height - 1)
                {
                    int16_t* dst_y_data = CT_IMAGE_DATA_PTR_16S(dst_y, x, y);
                    sobel3x3_calculate(src, x, y, dst_data, dst_y_data);
                });
    }
    else if (border.mode == VX_BORDER_MODE_REPLICATE)
    {
        CT_FILL_IMAGE_16S(return, dst_x,
                {
                    int16_t* dst_y_data = CT_IMAGE_DATA_PTR_16S(dst_y, x, y);
                    sobel3x3_calculate_replicate(src, x, y, dst_data, dst_y_data);
                });
    }
    else if (border.mode == VX_BORDER_MODE_CONSTANT)
    {
        vx_uint32 constant_value = border.constant_value;
        CT_FILL_IMAGE_16S(return, dst_x,
                {
                    int16_t* dst_y_data = CT_IMAGE_DATA_PTR_16S(dst_y, x, y);
                    sobel3x3_calculate_constant(src, x, y, constant_value, dst_data, dst_y_data);
                });
    }
    else
    {
        ASSERT_(return, "NOT IMPLEMENTED");
    }

    *p_dst_x = dst_x;
    *p_dst_y = dst_y;
}


static void sobel3x3_check(CT_Image src, CT_Image dst_x, CT_Image dst_y, vx_border_mode_t border)
{
    CT_Image dst_x_ref = NULL, dst_y_ref = NULL;

    ASSERT(src && dst_x && dst_y);

    ASSERT_NO_FAILURE(sobel3x3_create_reference_image(src, border, &dst_x_ref, &dst_y_ref));

    ASSERT_NO_FAILURE(
        if (border.mode == VX_BORDER_MODE_UNDEFINED)
        {
            ct_adjust_roi(dst_x,  1, 1, 1, 1);
            ct_adjust_roi(dst_x_ref, 1, 1, 1, 1);
            ct_adjust_roi(dst_y,  1, 1, 1, 1);
            ct_adjust_roi(dst_y_ref, 1, 1, 1, 1);
        }
    );

    EXPECT_EQ_CTIMAGE(dst_x_ref, dst_x);
    EXPECT_EQ_CTIMAGE(dst_y_ref, dst_y);
#if 0
    if (CT_HasFailure())
    {
        printf("=== SRC ===\n");
        ct_dump_image_info(src);
        printf("=== DST X ===\n");
        ct_dump_image_info(dst_x);
        printf("=== EXPECTED X ===\n");
        ct_dump_image_info(dst_x_ref);
        printf("=== DST Y ===\n");
        ct_dump_image_info(dst_y);
        printf("=== EXPECTED Y ===\n");
        ct_dump_image_info(dst_y_ref);
    }
#endif
}

typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    vx_border_mode_t border;
    int width, height;
} Filter_Arg;

#define SOBEL_PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_SMALL_SET, ARG, sobel3x3_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("lena", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ARG, sobel3x3_read_image, "lena.bmp")

TEST_WITH_ARG(Sobel3x3, testGraphProcessing, Filter_Arg,
    SOBEL_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_x_image = 0, dst_y_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;

    CT_Image src = NULL, dst_x = NULL, dst_y = NULL;
    vx_border_mode_t border = arg_->border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst_x_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst_x_image, VX_TYPE_IMAGE);

    dst_y_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst_y_image, VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxSobel3x3Node(graph, src_image, dst_x_image, dst_y_image);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &border, sizeof(border)));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    ASSERT_NO_FAILURE(dst_x = ct_image_from_vx_image(dst_x_image));
    ASSERT_NO_FAILURE(dst_y = ct_image_from_vx_image(dst_y_image));

    ASSERT_NO_FAILURE(sobel3x3_check(src, dst_x, dst_y, border));

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    ASSERT(node == 0);
    ASSERT(graph == 0);

    vxReleaseImage(&dst_x_image);
    vxReleaseImage(&dst_y_image);
    vxReleaseImage(&src_image);

    ASSERT(dst_x_image == 0);
    ASSERT(dst_y_image == 0);
    ASSERT(src_image == 0);
}

TEST_WITH_ARG(Sobel3x3, testImmediateProcessing, Filter_Arg,
    SOBEL_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_x_image = 0, dst_y_image = 0;

    CT_Image src = NULL, dst_x = NULL, dst_y = NULL;
    vx_border_mode_t border = arg_->border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    dst_x_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst_x_image, VX_TYPE_IMAGE);

    dst_y_image = ct_create_similar_image_with_format(src_image, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(dst_y_image, VX_TYPE_IMAGE);

    VX_CALL(vxSetContextAttribute(context, VX_CONTEXT_ATTRIBUTE_IMMEDIATE_BORDER_MODE, &border, sizeof(border)));

    VX_CALL(vxuSobel3x3(context, src_image, dst_x_image, dst_y_image));

    ASSERT_NO_FAILURE(dst_x = ct_image_from_vx_image(dst_x_image));
    ASSERT_NO_FAILURE(dst_y = ct_image_from_vx_image(dst_y_image));

    ASSERT_NO_FAILURE(sobel3x3_check(src, dst_x, dst_y, border));

    vxReleaseImage(&dst_x_image);
    vxReleaseImage(&dst_y_image);
    vxReleaseImage(&src_image);

    ASSERT(dst_x_image == 0);
    ASSERT(dst_y_image == 0);
    ASSERT(src_image == 0);
}

TESTCASE_TESTS(Sobel3x3, testNodeCreation, testGraphProcessing, testImmediateProcessing)
