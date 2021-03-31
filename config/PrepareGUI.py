import time
import sys
import os
import shutil
import re
import threading

global xlsxList

def DiffChange():
    jsonstr = "[\n\t{\n"
    jsonstr += '\t\t"name": "一键选中变更表格",\n'
    jsonstr += '\t\t"by_sheets": [\n'
    jsonstr += '\t\t\t{"file": "excel/quests.xlsx"},\n'
    jsonstr += '\t\t\t{"file": "excel/objects.xlsx"}\n'
    jsonstr += '\t\t],\n'
    jsonstr += '\t\t"default_selected": false,\n'
    jsonstr += '\t\t"style": "outline-primary"\n'
    jsonstr += '\t},\n'
    jsonstr += '\t{\n'
    jsonstr += '\t\t"name": "检查变化表格",\n'
    jsonstr += '\t\t"action": ["unselect_all", "script: diffchange", "script: delay", "reload"],\n'
    jsonstr += '\t\t"style": "outline-primary"\n'
    jsonstr += '\t},\n'
    jsonstr += '\t{\n'
    jsonstr += '\t\t"name": "Reload",\n'
    jsonstr += '\t\t"action": ["reload"],\n'
    jsonstr += '\t\t"style": "outline-primary"\n'
    jsonstr += '\t}\n'
    jsonstr += ']\n'
    open("custom-selector.json", "w", encoding="utf-8").write(jsonstr)

if __name__ == '__main__':
    DiffChange()