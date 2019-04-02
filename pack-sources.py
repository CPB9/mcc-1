#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import git
import os
import json
import sys
import tarfile

json_path = sys.argv[1]
js = json.load(open(json_path, 'r'))
src_dirs = js.get('subdirs', [])
#subprojects = [m['subproject'] for m in js.get('modules')]
subprojects = []
subproject_ignores = dict()
ignores = js.get('ignore_dirs', [])

for m in js.get('modules'):
    subprojects.append(m['subproject'])
    subproject_ignores[m['subproject']] = m.get('ignore_dirs', [])


archive = tarfile.open('mcc-src.tar.xz', mode='w:xz', dereference=True)


def pack_src(real_path, fake_path=None):
    name = real_path
    if fake_path is None:
        arcname = real_path
    else:
        arcname = fake_path
    arcname = os.path.join('mcc', arcname)
    #print('>', arcname)
    archive.add(name, arcname=arcname, recursive=False)


def visit_subproject_entry(entry, subprojects, ignores, visitor):
    if entry.path.endswith('.wrap'):
        f = open(entry.path, 'r')
        header = f.readline().strip() #header
        d = dict()
        for line in f.readlines():
            line = line.strip()
            if line != '':
                parts = line.split('=')
                d[parts[0].strip()] = parts[1].strip()
        sub_name = os.path.basename(entry.path)[:-5]
        if sub_name in subprojects:
            real_dir = os.path.join('subprojects', d['directory'])
            fake_dir = os.path.join('subprojects', sub_name)
            ignore_list = ignores[sub_name]
            if header == '[wrap-git]':
                traverse_git_subproject(real_dir, fake_dir, ignore_list, visitor)
            elif header == '[wrap-file]':
                traverse_file_subproject(real_dir, fake_dir, ignore_list, visitor)
            else:
                print('invalid header:', header)
    else:
        visitor(entry.path)


def visit_mcc_src_entry(entry, src_dirs, visitor):
    for d in src_dirs:
        if entry.path.startswith(d):
            visitor(entry.path)
            return


def visit_mcc_root_entry(entry, ignores, visitor):
    for i in ignores:
        if entry.path.startswith(i):
            return
    if entry.path.endswith('.json'):
        entry_base = os.path.basename(entry.path)
        json_base = os.path.basename(json_path)
        if entry_base != json_base:
            return
    visitor(entry.path)


def is_ignored(path, ignore_list):
    for i in ignore_list:
        if path.startswith(i):
            return True
    return False


def traverse_file_subproject(real_dir, fake_dir, ignore_list, visitor):
    for root, dirs, files in os.walk(real_dir):
        for fname in files:
            real_path = os.path.join(root, fname)
            if (not fname.endswith('.wrap')) and (not is_ignored(real_path[len(real_dir)+1:], ignore_list)):
                fake_path = real_path.replace(real_dir, fake_dir, 1)
                visitor(real_path, fake_path)


def traverse_git_subproject(real_dir, fake_dir, ignore_list, visitor):
    repo = git.Repo(real_dir)
    for entry in repo.tree().traverse():
        if (not entry.path.endswith('.wrap')) and (not is_ignored(entry.path, ignore_list)):
            visitor(os.path.join(real_dir, entry.path), os.path.join(fake_dir, entry.path))


def traverse_mcc_repo(path, src_dirs, subprojects, ignores, subproject_ignores, visitor):
    mcc_repo = git.Repo(path)
    for entry in mcc_repo.tree().traverse():
        if entry.path.startswith('src') or entry.path.startswith('plugins'):
            visit_mcc_src_entry(entry, src_dirs, visitor)
        elif entry.path.startswith('subprojects'):
            visit_subproject_entry(entry, subprojects, subproject_ignores, visitor)
        else:
            visit_mcc_root_entry(entry, ignores, visitor)


traverse_mcc_repo('./', src_dirs, subprojects, ignores, subproject_ignores, pack_src)

extra_files = js.get('extra_files', [])
for fname in extra_files:
    pack_src(fname)

