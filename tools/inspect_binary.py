"""Inspect PE exports/imports without loading executable code."""
from pathlib import Path
import argparse,hashlib,json,struct

ROOT=Path(__file__).resolve().parents[1]
def inspect(data):
    u16=lambda p:struct.unpack_from('<H',data,p)[0]
    u32=lambda p:struct.unpack_from('<I',data,p)[0]
    pe=u32(0x3c);assert data[pe:pe+4]==b'PE\0\0'
    machine=u16(pe+4);count=u16(pe+6);opt=pe+24
    names={0x14c:('I386',0x10b,96),0x8664:('AMD64',0x20b,112),0xaa64:('ARM64',0x20b,112)}
    assert machine in names, f'Unsupported PE machine: {machine:#x}'
    machine_name,magic,directory_offset=names[machine]
    assert u16(opt)==magic, 'PE machine and optional header disagree'
    directories=opt+directory_offset
    table=opt+u16(pe+20);sections=[]
    for i in range(count):
        p=table+i*40;sections.append((u32(p+12),max(u32(p+8),u32(p+16)),u32(p+20)))
    def offset(rva):
        for start,size,raw in sections:
            if start<=rva<start+size:return raw+rva-start
        raise ValueError(f'Unmapped RVA: {rva}')
    def string(rva):
        p=offset(rva);return data[p:data.index(b'\0',p)].decode('ascii')
    export=offset(u32(directories));export_names=offset(u32(export+32))
    exports=[string(u32(export_names+i*4)) for i in range(u32(export+24))]
    required={'isUnicode','setInfo','getName','getFuncsArray','beNotified','messageProc','GetLexerCount','GetLexerName','CreateLexer'}
    assert required<=set(exports),required-set(exports)
    imports=[];p=offset(u32(directories+8))
    while any(data[p:p+20]):imports.append(string(u32(p+12)));p+=20
    resource_rva=u32(directories+16)
    assert resource_rva, 'Windows resources are missing'
    resource_base=offset(resource_rva)
    def children(relative):
        p=resource_base+relative
        count=u16(p+12)+u16(p+14)
        return [(u32(p+16+i*8),u32(p+20+i*8)) for i in range(count)]
    def resource(kind, identifier=None):
        entry=next(value for key,value in children(0) if key==kind)
        names=children(entry&0x7fffffff)
        entry=next(value for key,value in names if identifier is None or key==identifier)
        entry=children(entry&0x7fffffff)[0][1]
        p=resource_base+entry
        start=offset(u32(p));return data[start:start+u32(p+4)]
    version_data=resource(16)
    position=version_data.index(b'\xbd\x04\xef\xfe')
    fixed=struct.unpack_from('<13I',version_data,position)
    version='.'.join(str(n) for n in (fixed[2]>>16,fixed[2]&65535,fixed[3]>>16,fixed[3]&65535))
    embedded=resource(10,101)
    dark=resource(10,102)
    report=dict(passed=True,machine=machine_name,file_version=version,embedded_light_sha256=hashlib.sha256(embedded).hexdigest(),embedded_dark_sha256=hashlib.sha256(dark).hexdigest(),
                dll_sha256=hashlib.sha256(data).hexdigest(),exports=exports,imports=imports,
                checked_on='2026-10-31',inspection='PE tables only; no code execution')
    return report

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--dll',type=Path,default=ROOT/'build/x64/ForeLexer.dll')
    parser.add_argument('--report',type=Path,default=ROOT/'qa/binary-validation.json')
    args=parser.parse_args()
    report=inspect(args.dll.read_bytes())
    assert report['embedded_light_sha256']==hashlib.sha256((ROOT/'styles/ForeLexer-Light.xml').read_bytes()).hexdigest()
    assert report['embedded_dark_sha256']==hashlib.sha256((ROOT/'styles/ForeLexer-Dark.xml').read_bytes()).hexdigest()
    args.report.parent.mkdir(parents=True,exist_ok=True)
    args.report.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8',newline='\n')
    print('PE:',report['machine'],'version:',report['file_version'],'required exports and embedded preset verified; imports:',', '.join(report['imports']))
if __name__=='__main__':main()
