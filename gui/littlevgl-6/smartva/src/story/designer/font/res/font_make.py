"""

this file is to change split the lang.txt file to
    lang.h  and every kind of language bin file

"""
def write_h_info(file):
    head_info = "/*\n\
*************************************************************************\n\
*File   : lang.h\n\
*Moudle	: language compiler\n\
*************************************************************************\n\
*/\
\n\
#ifndef __LANG__H__\n\
#define __LANG__H__\n\n\n"
    end_info = "\n#endif //__LANG__H__\n"
    yield file.write(head_info)
    yield file.write(end_info)

"""
def get_git_log():
    val = os.popen("git log").read()
    commit_id = val.split()[1]
    #print(commit)
    return commit_id
"""

def move_h_file():
    src_path = "./lang.h"
    dst_path = "../../../../common/middle/va_lvgl/va_font/lang.h"
    shutil.move(src_path, dst_path)

# remove all the bin file in this dir
def remove_all_bin_file():
    print("first remove the all old language file .....")
    lenx = os.listdir()
    for item in lenx:
        if item.endswith(".bin"):
            print("remove file:", item)
            os.remove(item)

# read the first line to check the lang.txt  how mang language kind in .
# and then create a lang file to every language
def create_bin_file():
    print("\nnow create language file .....")
    line_cnt = -1
    file_list = []
    with open("lang.txt", mode="r", encoding="utf-16") as lang_txt_f:
        line = lang_txt_f.readline()
        line = line.strip().split("\t")
        #print(line)
        for item in line:
            line_cnt += 1
            if line_cnt == 0:
                continue
            item = item + ".bin"
            f = open(item, mode="w", encoding="utf-8")
            #print("create file:", item)
            file_list.append(item)
            f.close()

    print("create file:", file_list)
    return file_list

# start running
import os,shutil
remove_all_bin_file()
file_list = create_bin_file();
# to create the lang.h for C use
print("\nnow create lang.h .....")
with open("lang.txt", mode="r", encoding="utf-16") as lang_txt, \
     open("lang.h", mode="w", encoding="utf-8") as h_lang:
    define_cnt = -1
    define_cnt_use = -1
    fn = write_h_info(h_lang)
    fn.__next__()
    for line in lang_txt:
        define_cnt += 1         # first word is "ID" no use here
        if define_cnt == 0:
            continue
        line = line.strip()     # ignore the empty line
        if len(line) == 0:
            continue
        define_cnt_use += 1
        line_first_word = line.split("\t")[0]
        #print(line_first_word)
        line_first_word = line_first_word.upper()
        str_macro = "#define LANG_" + line_first_word + "\t" + str(define_cnt_use).strip() + "\n"
        h_lang.write(str_macro)
        #bin_lang.write(str_macro)
        #print(str_macro)
    fn.__next__()

lang_txt_f = open("lang.txt", mode="r", encoding="utf-16")
#content = lang_txt_f.readlines()
lang_cnt = 0
for item in file_list:
    lang_cnt += 1
    lang_txt_f.seek(0, 0)
    line_cnt = -1
    f = open(item, mode="a", encoding="utf-8")
    for line in lang_txt_f:
        line_cnt += 1         # first word is "ID" no use here
        if line_cnt == 0:
            continue
        line = line.strip()     # ignore the empty line
        if len(line) == 0:
            continue
        line_word = line.split("\t")[lang_cnt]
        f.write(line_word)
        f.write("\n")
        #print(line_word)
    f.close()
lang_txt_f.close()
move_h_file()
print("\n...\npy: make the language resourse finish!!!\n")
