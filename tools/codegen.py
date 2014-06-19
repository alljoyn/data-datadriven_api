# Copyright (c) 2014, AllSeen Alliance. All rights reserved.
#
#    Permission to use, copy, modify, and/or distribute this software for any
#    purpose with or without fee is hereby granted, provided that the above
#    copyright notice and this permission notice appear in all copies.
#
#    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
#    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
#    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
#    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
#    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
#    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
#    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

import os
import re
import subprocess

def __codegen_detect(env):
    """Detect the AJN code generator.

    First searches for the code generator in the OS's PATH, then tries to
    resolve it using the AJREPOPATH scons environment variable which might have
    been derived from the OS's AJ_ROOT environment variable."""
    e = os.environ.copy()
    ajcg = env.WhereIs('ajcodegen.py', e['PATH'])
    if not ajcg:
        ajcg_root = env.Dir(env['AJREPOPATH'] + '/devtools/codegen').abspath
        ajcg = ajcg_root + '/build/scripts-2.7/ajcodegen.py'
        if not os.path.isfile(ajcg):
            raise Exception('No code generator found, aborting!')
        if os.path.isdir(ajcg_root + '/build/lib.linux-' + env['CPU'] + '-2.7'):
            # for 64-bit
            e['PYTHONPATH'] = ajcg_root + '/build/lib.linux-' + env['CPU'] + '-2.7'
        elif os.path.isdir(ajcg_root + '/build/lib'):
            # for 32-bit
            e['PYTHONPATH'] = ajcg_root + '/build/lib'
        else:
            raise Exception('No code generator found, aborting!')
    return ajcg, e

def __codegen(target, source, env):
    """Generate Data-driven API C++ code for the given sources."""
    target_cc = []
    target_h = []
    lang = 'ddcpp'
    for xml in source:
        output = target[0].get_dir()
        try:
            os.makedirs(output.abspath)
        except OSError:
            pass
        ajcg, osenv = __codegen_detect(env)
        cmd = [ajcg, '-t', lang, '-p', output.abspath, xml.srcnode().abspath]
        p = subprocess.Popen(cmd, env = osenv)
        p.wait()
        if 0 != p.returncode:
            raise Exception('Code generation failed, aborting!')
    return None

def __codegen_emitter(target, source, env):
    """Create a list of output files for the interfaces defined in the
    sources."""
    target = []
    for xml in source:
        tgt4src = []
        content = xml.get_contents()
        intf_name_re = re.compile('<interface name="[^"]*\.([^."]*)">')
        for match in intf_name_re.finditer(content):
            name = match.group(1)
            for suffix in [ 'TypeDescription', 'Interface', 'Proxy' ]:
                for ext in ['h', 'cc']:
                    tgt4src.append('generated/' + name + suffix + '.' + ext)
        if not tgt4src:
            raise Exception('Failed to detect interface name in %s, aborting!' % xml)
        target.extend(tgt4src)
    return target, source

def tool(env):
    builder = env.Builder(action = __codegen,
                          emitter = __codegen_emitter,
                          src_suffix = '.xml')
    env.Append(BUILDERS = { 'CodeGen' : builder })
