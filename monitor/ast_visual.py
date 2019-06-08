from ete3 import *
import json
import math
with open("./main.json","r") as f:
    load_dict = json.load(f)

t = Tree()

def traverse_dict(d, node):
    for key, value in d.items():
        if(isinstance(value, dict)):
            A = node.add_child(name=key)
            traverse_dict(value, A)
        else:
            if(node.name.find("terminal") != -1):
                node.name = key + ": " + value
            else:
                node.name += "\n" + key + ": " + value

traverse_dict(load_dict, t)

def get_nodes(node):
    l = 0
    for n in node.traverse():
        l += 1
    return l

print ("Original tree:")
print (t.get_ascii(show_internal=True))
ts = TreeStyle()
ts.show_leaf_name = False
def my_layout(node):
    fstle = "normal"
    pos = "branch-top"
    bd = False
    fgc = "#4169e1"
    if(get_nodes(node) == 1):
        fstle = "italic"
        pos = "branch-right"
        bd = True
        fgc = "#000000"
    if(node.name.find("\n") != -1):
        a = node.name[0: node.name.find("\n")]
        b = node.name[node.name.find("\n") + 1: len(node.name)]    
        F = TextFace(a, fgcolor=fgc, fsize=12, fstyle=fstle, bold=bd)
        G = TextFace(b, fgcolor=fgc, fsize=12, fstyle=fstle, bold=False)
        add_face_to_node(F, node, column=0, position=pos)
        add_face_to_node(G, node, column=0, position=pos)
    else:
        F = TextFace(node.name, fgcolor=fgc, fsize=12, fstyle=fstle, bold=bd)
        add_face_to_node(F, node, column=0, position=pos)

ts.scale = 20
ts.layout_fn = my_layout
max = get_nodes(t)
for n in t.traverse():
    l = get_nodes(n)
    width = l / (max - 1) * 5
    style = NodeStyle()
    style["shape"] = "circle"
    style["size"] = 7
    style["fgcolor"] = "#000000"
    style["vt_line_color"] = "#000000"
    style["hz_line_color"] = "#000000"
    style["vt_line_type"] = 0
    style["hz_line_type"] = 0
    style["hz_line_width"] = width
    style["vt_line_width"] = width
    dist = n.name.find("\n")
    if(dist == -1):
        dist = len(n.name)
    n.dist = dist*0.6
    if(l == 1):
        n.dist = 3
    n.set_style(style)

t.show(tree_style=ts)
t.render("mytree.png", tree_style=ts)
