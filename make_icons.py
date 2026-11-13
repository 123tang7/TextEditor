"""生成 TextEditor 工程所需的 SVG 图标（线性风格，24x24）。"""
import os

DARK = "#2F3E46"
BLUE = "#2D6CDF"
RED = "#E53935"
ORANGE = "#F08C00"

ICONS = {
    # 文件
    "new": f'''
<path d="M6 3h7l5 5v13H6z" stroke="{DARK}"/>
<path d="M13 3v5h5" stroke="{DARK}"/>''',
    "open": f'''
<path d="M3 7h6l2 2h9v3H3z" stroke="{DARK}"/>
<path d="M3 12h18l-2 8H5z" stroke="{DARK}"/>''',
    "save": f'''
<path d="M4 4h13l3 3v13H4z" stroke="{DARK}"/>
<path d="M8 4h7v5H8z" stroke="{DARK}"/>
<path d="M8 14h8v6H8z" stroke="{DARK}"/>''',
    "saveas": f'''
<path d="M3 4h12l3 3v9H3z" stroke="{DARK}"/>
<path d="M6 4h6v4H6z" stroke="{DARK}"/>
<path d="M15 17l4-4 3 3-4 4h-3z" stroke="{BLUE}"/>''',
    "exit": f'''
<path d="M13 4H5v16h8" stroke="{DARK}"/>
<path d="M11 12h10" stroke="{RED}"/>
<path d="M17 8l4 4-4 4" stroke="{RED}"/>''',
    # 编辑
    "undo": f'''
<path d="M5 10h9a5 5 0 1 1 0 10h-3" stroke="{DARK}"/>
<path d="M9 6l-4 4 4 4" stroke="{DARK}"/>''',
    "redo": f'''
<path d="M19 10H10a5 5 0 1 0 0 10h3" stroke="{DARK}"/>
<path d="M15 6l4 4-4 4" stroke="{DARK}"/>''',
    "cut": f'''
<circle cx="6.5" cy="7" r="2.4" stroke="{DARK}"/>
<circle cx="6.5" cy="17" r="2.4" stroke="{DARK}"/>
<path d="M8.4 8.5L20 18.5M8.4 15.5L20 5.5" stroke="{DARK}"/>''',
    "copy": f'''
<path d="M9 9h11v11H9z" stroke="{DARK}"/>
<path d="M15 9V4H4v11h5" stroke="{DARK}"/>''',
    "paste": f'''
<path d="M7 5h10v16H7z" stroke="{DARK}"/>
<path d="M10 3h4v4h-4z" stroke="{DARK}"/>
<path d="M10 12h4M10 16h4" stroke="{BLUE}"/>''',
    "selectall": f'''
<rect x="4" y="4" width="16" height="16" stroke="{DARK}" stroke-dasharray="3 2.5"/>''',
    "find": f'''
<circle cx="10.5" cy="10.5" r="6" stroke="{DARK}"/>
<path d="M15 15l5.5 5.5" stroke="{DARK}"/>''',
    "replace": f'''
<circle cx="10" cy="10" r="5.5" stroke="{DARK}"/>
<path d="M14 14l6 6" stroke="{DARK}"/>
<path d="M7.5 10h5M10.5 8l2 2-2 2" stroke="{BLUE}"/>''',
    # 格式
    "font": f'''
<path d="M5.5 20L12 4l6.5 16" stroke="{DARK}"/>
<path d="M8.2 14.5h7.6" stroke="{DARK}"/>''',
    "fontcolor": f'''
<path d="M5.5 16L11 4l5.5 12" stroke="{DARK}"/>
<path d="M7.5 12h7" stroke="{DARK}"/>
<rect x="4.5" y="18.5" width="14" height="3" fill="{RED}"/>''',
    "background": f'''
<path d="M12 3l8 8-8 8-8-8z" stroke="{DARK}"/>
<path d="M8 7l8 8" stroke="{ORANGE}" stroke-width="2.2"/>
<path d="M20 17c1.2 1.8 1.8 2.7 1.8 3.4a1.8 1.8 0 1 1-3.6 0c0-.7.6-1.6 1.8-3.4z" stroke="{BLUE}"/>''',
    "wrap": f'''
<path d="M4 7h13a3.5 3.5 0 0 1 0 7H9" stroke="{DARK}"/>
<path d="M12 11l-3 3 3 3" stroke="{BLUE}"/>''',
    "linenumbers": f'''
<path d="M9 4L7 20M17 4l-2 16" stroke="{DARK}"/>
<path d="M4 9h16M3 15h16" stroke="{DARK}"/>''',
    "toolbar": f'''
<rect x="3" y="4" width="18" height="16" rx="1.5" stroke="{DARK}"/>
<path d="M3 9h18" stroke="{DARK}"/>
<path d="M6 6.5h2M10 6.5h2M14 6.5h2" stroke="{BLUE}" stroke-width="2"/>''',
    "statusbar": f'''
<rect x="3" y="4" width="18" height="16" rx="1.5" stroke="{DARK}"/>
<path d="M3 16h18" stroke="{DARK}"/>
<path d="M6 19h5" stroke="{BLUE}" stroke-width="2"/>''',
    "about": f'''
<circle cx="12" cy="12" r="9" stroke="{DARK}"/>
<circle cx="12" cy="7.8" r="1.1" fill="{DARK}" stroke="none"/>
<path d="M12 11v6" stroke="{DARK}"/>''',
}

TPL = ('<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" '
       'fill="none" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">\n'
       '%s\n</svg>\n')

out = os.path.join(os.path.dirname(os.path.abspath(__file__)), "icons")
os.makedirs(out, exist_ok=True)
names = []
for name, body in ICONS.items():
    with open(os.path.join(out, name + ".svg"), "w", encoding="utf-8") as f:
        f.write(TPL % body.strip())
    names.append(name)
print("generated:", len(names))
print(" ".join(sorted(names)))
