#!/bin/python3

import os
import sys
from PIL import Image
from reportlab.lib.pagesizes import A4
from reportlab.pdfgen import canvas

def convert_png_to_pdf(png_file, pdf_file, page_size):
    # 打开PNG图像
    img = Image.open(png_file)
    
    # 创建PDF文件
    c = canvas.Canvas(pdf_file, pagesize=page_size)
    
    # 获取PDF页面尺寸
    width, height = page_size
    
    # 计算图像缩放比例
    scale_x = width / img.width
    scale_y = height / img.height
    scale = min(scale_x, scale_y)

    # 计算绘制图像的大小和位置
    img_width = img.width * scale
    img_height = img.height * scale
    x_offset = (width - img_width) / 2
    y_offset = (height - img_height) / 2

    # 将图像绘制到PDF文件中
    c.drawImage(png_file, x_offset, y_offset, width=img_width, height=img_height)
    
    # 保存PDF文件
    c.save()

def convert_all_pngs_to_pdf(directory, output_pdf, page_size):
    # 创建PDF文件
    c = canvas.Canvas(output_pdf, pagesize=page_size)
    
    # 获取PDF页面尺寸
    width, height = page_size
    
    # 用于计算累积高度

    # 获取指定目录下的所有文件
    for filename in sorted(os.listdir(directory)):
        if filename.endswith(".png") and "_Phi_" in filename:
            # 构造PNG文件的完整路径
            png_file = os.path.join(directory, filename)
            
            # 打开PNG图像
            img = Image.open(png_file)

            # 计算图像缩放比例
            scale_x = width / img.width
            scale_y = height / img.height
            scale = min(scale_x, scale_y)

            # 计算绘制图像的大小和位置
            img_width = img.width * scale
            img_height = img.height * scale
            x_offset = (width - img_width) / 2

            # 累积高度
            y_offset = height/2-img_height/2

            # 将图像绘制到PDF文件中
            c.drawImage(png_file, x_offset, y_offset, width=img_width, height=img_height)

            c.setFont("Helvetica", 10)
            c.drawString(10, height - 50, "Positive Eta")

            page_number_text = f"Page {c.getPageNumber()}"
            c.setFont("Helvetica", 10)
            c.drawString(10, 10, page_number_text)
            # 添加分页
            c.showPage()


            print(f"Added {png_file} to {output_pdf}")

    # 保存PDF文件
    c.save()

# 指定页面尺寸
page_size = (500,400)

# 从命令行参数中获取输入和输出文件路径
if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python png2pdf.py <input_directory> <output_pdf>")
        sys.exit(1)

    input_directory = sys.argv[1]
    output_pdf = sys.argv[2]

    convert_all_pngs_to_pdf(input_directory, output_pdf, page_size)
