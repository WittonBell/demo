# -*- coding: utf-8 -*-
import binascii

import fitz_old as fitz
import sys

# 构建需要替换的字体，Key为源PDF中使用的字体，Value为要替换为的系统中的字体文件路径
dict_new_font = {}
# 宋体替换为系统的宋体
dict_new_font['SimSun'] = 'c:/windows/fonts/simsun.ttc'
# 黑体替换为系统的黑体
dict_new_font['SimHei'] = 'c:/windows/fonts/simhei.ttf'
# 楷体及楷体GB2312替换为系统的楷体
dict_new_font['SimKai'] = 'c:/windows/fonts/simkai.ttf'

def replace_page(page: fitz.Page):
    span_list = []
    info = page.get_text('dict')
    for block in info['blocks']:
        lines = block.get('lines')
        if lines is None:
            continue
        for line in lines:
            for span in line['spans']:
                font_name = span['font']
                name = font_name.lower()
                if name.startswith('simsun'):
                    font_name = 'SimSun'
                elif name.startswith('simhei'):
                    font_name = 'SimHei'
                elif name.startswith('simkai'):
                    font_name = 'SimKai'
                else:
                    continue
                span['font'] = font_name
                page.add_redact_annot(span['bbox'])
                span_list.append(span)
    page.apply_redactions()
    for target in span_list:
        text = target['text']
        font_name = target['font']
        page.insert_text(target['origin'], text, fontsize=target['size'], fontname=font_name,
                         fontfile=dict_new_font[font_name])

def decode(s):
    try:
        return s.encode('latin1').decode('gbk')
    except:
        return s.encode('utf-16').decode("utf-16")

def show(doc, field):
    txt = doc.metadata[field]
    print(field, " : ", decode(txt))

def string_to_ascii(s):
    l = []
    for c in s:
        cc = ord(c)
        x = bin(cc)
        l.append(x)
    return l

def show_outline(outline):
    while outline:
        t = outline.title
        t1 = t.encode('utf-16be').decode('utf-16le')
        print(t1)
        if outline.down is not None:
            show_outline(outline.down)
        outline = outline.next

def replace_font(doc_path, save_path):
    doc = fitz.open(doc_path)
    title = doc.metadata['title']
    author = doc.metadata['author']
    subject = doc.metadata['subject']
    keywords = doc.metadata['keywords']
    creator = doc.metadata['creator']
    producer = doc.metadata['producer']

    print("")
    show(doc, 'title')
    show(doc, 'author')
    show(doc, 'subject')
    show(doc, 'keywords')
    show(doc, 'creator')
    show(doc, 'producer')
    show_outline(doc.outline)
    for page in doc:
        lst = page.get_contents()
        for xref in lst:
            stream = doc.xref_stream(xref)
            if stream is not None:
                pass

    doc.close()
    return

    n = len(doc)
    for page in doc:
        replace_page(page)
        print(f"{page.number}/{n}")

    print("清理字体")
    # 清理使用的字体
    doc.subset_fonts()
    print("保存文件")
    # 保存时，清理没使用的对象，减少文件大小
    doc.ez_save(save_path, clean=True)
    doc.close()
    print("完成")

def main():
    if len(sys.argv) < 3:
        print(f"需要传入参数：格式：{sys.argv[0]} <源pdf> <目标pdf>")
        return
    replace_font(sys.argv[1], sys.argv[2])

if __name__ == "__main__":
    main()
