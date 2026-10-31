"""Render documented code previews from the actual Fore lexer, not a UI recording.
Run the compiled qa/preview-tokens.cpp helper first or pass its executable path.
"""
from pathlib import Path
import argparse
import hashlib
import json
import subprocess
import xml.etree.ElementTree as ET
from PIL import Image, ImageDraw, ImageFont

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'docs/media'
FONT=Path('C:/Windows/Fonts/consola.ttf')
BOLD=Path('C:/Windows/Fonts/consolab.ttf')
UI=Path('C:/Windows/Fonts/segoeui.ttf')
UIB=Path('C:/Windows/Fonts/segoeuib.ttf')
LANG='en'
TRANSLATIONS={
 'Your code. In the right light.':'Ваш код. В подходящей теме.',
 'Automatic light / dark palettes. Your custom colors stay yours.':'Светлая и тёмная темы переключаются автоматически. Ваши цвета сохраняются.',
 'See the structure. Keep your place.':'Структура кода — перед глазами.',
 'Collapse method bodies while keeping their declarations in view.':'Сворачивайте тела методов, оставляя на экране их заголовки.',
 'Real Fore, from the community.':'Настоящий Fore из открытого проекта.',
 'CUSTOM COLORS':'СВОИ ЦВЕТА', 'DARK PALETTE':'ТЁМНАЯ ПАЛИТРА', 'LIGHT PALETTE':'СВЕТЛАЯ ПАЛИТРА',
 'Lexer-rendered preview · Foresight / Prognoz Platform Fore · not a screen recording':'Превью по разметке лексера · Foresight / Prognoz Platform Fore · не запись окна редактора'
}
def translated(text):
    return TRANSLATIONS.get(text,text) if LANG=='ru' else text

def output_image(name):
    path=Path(name)
    return OUT/(path.stem+('.ru' if LANG=='ru' else '')+path.suffix)


def font(path,size):
    return ImageFont.truetype(str(path),size)

def parse_file(helper,path):
    raw=path.read_bytes()
    data=json.loads(subprocess.check_output([str(helper),str(path)]))
    assert len(data['styles'])==len(raw)
    lines=[]; offset=0
    for row in raw.splitlines(keepends=True):
        chars=[]; column=0
        for c in row.decode('utf-8'):
            style=data['styles'][offset];offset+=len(c.encode('utf-8'))
            if c in '\r\n':continue
            expanded=' '*(4-column%4) if c=='\t' else c
            chars.extend((letter,style) for letter in expanded);column+=len(expanded)
        lines.append(chars)
    return lines,data['folds']

def palette(theme):
    nodes=ET.parse(ROOT/f'styles/ForeLexer-{theme}.xml').findall('.//WordsStyle')
    return {int(n.get('styleID')):'#'+n.get('fgColor') for n in nodes}

def visible_rows(lines,folds,first,last,collapsed):
    result=[]; i=first-1
    while i<min(last,len(lines)):
        text=''.join(c for c,s in lines[i]); result.append(i)
        is_method=any(word in text for word in ('Public Constructor','Public Sub','Public Function'))
        if collapsed and is_method and folds[i]&0x2000:
            level=folds[i]&0xFFF;i+=1
            while i<len(lines) and folds[i]&0xFFF>level:i+=1
        else:i+=1
    return result

def frame(lines,folds,theme,title,subtitle,filename,first,last,collapsed=False,custom=False):
    width=1440;height=285+(last-first+1)*28
    background='#0F172A';panel='#1E1E1E' if theme=='Dark' else '#FFFFFF'
    muted='#8E9BAE' if theme=='Dark' else '#697586';border='#334155' if theme=='Dark' else '#DAE2EB'
    colors=palette(theme)
    if custom: colors.update({1:'#FFD166',7:'#8BE28B',10:'#8BE28B',5:'#FFA1B5',6:'#FFA1B5'})
    canvas=Image.new('RGB',(width,height),background); draw=ImageDraw.Draw(canvas)
    draw.text((38,22),'FORE  /  NOTEPAD++',font=font(UIB,22),fill='#67E8E0')
    draw.text((38,57),translated(title),font=font(UIB,32),fill='#F8FAFC')
    draw.text((38,101),translated(subtitle),font=font(UI,19),fill='#B7C5D8')
    draw.rounded_rectangle((24,143,width-24,height-57),radius=14,fill=panel,outline=border,width=1)
    draw.text((49,156),filename,font=font(UI,18),fill=muted)
    mode='CUSTOM COLORS' if custom else ('DARK PALETTE' if theme=='Dark' else 'LIGHT PALETTE')
    draw.text((width-225,157),translated(mode),font=font(UIB,15),fill=muted)
    draw.line((40,191,width-40,191),fill=border,width=1)
    regular=font(FONT,21);bold=font(BOLD,21);cell=regular.getlength('M')
    visible=visible_rows(lines,folds,first,last,collapsed)
    for r,i in enumerate(visible):
        y=203+r*28
        draw.text((48,y),str(i+1).rjust(3),font=font(FONT,17),fill=muted)
        header=bool(folds[i]&0x2000)
        if header:
            symbol='+' if collapsed and any(k in ''.join(c for c,s in lines[i]) for k in ('Public Constructor','Public Sub','Public Function')) else '−'
            draw.text((93,y),symbol,font=font(FONT,18),fill=muted)
        x=120
        for c,style in lines[i]:
            draw.text((x,y),c,font=bold if style==1 else regular,fill=colors[style])
            x+=cell
    draw.text((38,height-39),translated('Lexer-rendered preview · Foresight / Prognoz Platform Fore · not a screen recording'),font=font(UI,16),fill='#94A3B8')
    return canvas

def gif(name,frames,durations):
    # A shared palette keeps the standard role colors exact in the GIF.
    colors=[]
    for im in frames:
        colors.extend(color for count,color in im.getcolors(im.width*im.height) if count>300)
    colors=list(dict.fromkeys(colors))[:64]
    for foreground in list(colors):
        for background in ((255,255,255),(30,30,30),(15,23,42)):
            for part in (.25,.5,.75):
                colors.append(tuple(round(a*part+b*(1-part)) for a,b in zip(foreground,background)))
    colors=list(dict.fromkeys(colors))[:256]; colors += [(0,0,0)]*(256-len(colors))
    swatch=Image.new('P',(1,1));swatch.putpalette([c for rgb in colors for c in rgb])
    converted=[im.quantize(palette=swatch,dither=Image.Dither.NONE) for im in frames]
    converted[0].save(output_image(name),save_all=True,append_images=converted[1:],duration=durations,loop=0,optimize=False,disposal=2)

def main():
    global LANG
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--lexer',type=Path,required=True)
    parser.add_argument('--language',choices=['en','ru'],default='en')
    args=parser.parse_args();LANG=args.language
    OUT.mkdir(exist_ok=True)
    lines,folds=parse_file(args.lexer.resolve(),ROOT/'examples/Fore-demo.fore')
    real,real_folds=parse_file(args.lexer.resolve(),ROOT/'examples/third-party/mcp-data-sources.fore')
    frames=[frame(real,real_folds,theme,'Your code. In the right light.',
        'Automatic light / dark palettes. Your custom colors stay yours.','mcp-data-sources.fore · excerpt',15,43,custom=custom)
        for theme,custom in [('Light',False),('Dark',False),('Dark',True)]]
    for name,im in zip(['light.png','dark.png','custom.png'],frames):im.save(output_image(name))
    gif('themes.gif',frames,[3000,3000,3000])
    folding=[frame(lines,folds,'Dark','See the structure. Keep your place.',
        'Collapse method bodies while keeping their declarations in view.','Fore-demo.fore',22,41,collapsed=value)
        for value in [False,True]]
    folding[1].save(output_image('folded.png'));gif('folding.gif',folding,[2800,2800])
    real,real_folds=parse_file(args.lexer.resolve(),ROOT/'examples/third-party/mcp-data-sources.fore')
    im=frame(real,real_folds,'Dark','Real Fore, from the community.',
        'MCP_GetDataSources · ForesightPlatform/fap_mcp_server · MIT · Evgeny Bogdanov',
        'mcp-data-sources.fore · excerpt',15,43)
    im.save(output_image('community-example.png'))
    report={'method':'Actual fore::analyze results and XML palette colors, rendered with Pillow. No simulated Notepad++ UI.',
        'fonts':'Windows Consolas / Segoe UI; font files are not distributed.',
        'gifs':{},'sha256':{}}
    for path in sorted(OUT.iterdir()):
        if path.suffix in ('.gif','.png'):
            report['sha256'][path.name]=hashlib.sha256(path.read_bytes()).hexdigest()
            with Image.open(path) as asset:
                if path.suffix=='.gif':report['gifs'][path.name]={'frames':asset.n_frames,'size':asset.size,'bytes':path.stat().st_size}
    (OUT/'render-validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps(report['gifs']))

if __name__=='__main__':main()
