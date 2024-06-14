#!/bin/env python3

import sys
from PIL import Image
from reportlab.lib.pagesizes import letter
from reportlab.pdfgen import canvas

def convert_png_to_pdf(png_file, pdf_file):
    # 打开PNG图像
    img = Image.open(png_file)
    
    # 创建PDF文件
    c = canvas.Canvas(pdf_file, pagesize=letter)
    
    # 获取PDF页面尺寸
    width, height = letter

    # 计算图像缩放比例
    scale_x = width / img.width
    scale_y = height / img.height
    scale = min(scale_x, scale_y)

    # 计算绘制图像的位置和大小
    img_width = img.width * scale
    img_height = img.height * scale
    x_offset = (width - img_width) / 2
    y_offset = (height - img_height) / 2

    # 将图像绘制到PDF文件中
    c.drawImage(png_file, x_offset, y_offset, width=img_width, height=img_height)
    
    # 保存PDF文件
    c.save()

# 从命令行参数中获取输入和输出文件路径
if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python png2pdf.py <input_png_file> <output_pdf_file>")
        sys.exit(1)

    input_png_file = sys.argv[1]
    output_pdf_file = sys.argv[2]

    convert_png_to_pdf(input_png_file, output_pdf_file)

