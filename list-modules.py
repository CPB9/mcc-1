#!/usr/bin/env python

from __future__ import print_function

import sys
import json

command = sys.argv[1]
fname = sys.argv[2]

mods = json.load(open(fname, 'r'))

if command == 'subdirs':
    try:
        for d in mods['subdirs']:
            print(d)
    except KeyError:
        pass
elif command == 'modules':
    try:
        mod_descs = mods['modules']
    except KeyError:
        sys.exit(0)

    for m in mod_descs:
        sub_name = m.get('subproject', '')
        dep_name = m.get('dependency', '')
        dvar = m.get('dep_var', '')
        try:
            libs = ','.join(m['libs'])
        except KeyError:
            libs = ''
        try:
            tools = ','.join(m['tools'])
        except KeyError:
            tools = ''
        try:
            opts = ','.join(m['default_options'])
        except KeyError:
            opts = ''
        print('|'.join([dep_name, sub_name, dvar, libs, tools, opts]))
elif command == 'resources':
        resources = mods.get('replace_resources', {})
        print(json.dumps(resources))
