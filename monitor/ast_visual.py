from ete3 import *
import json
import math
with open("./main.json","r") as f:
    load_dict = json.load(f)

t = Tree()

def traverse_dict(d, node):
    for key, value in d.items():
        A = node.add_child(name=key)
        if(isinstance(value, dict)):
            traverse_dict(value, A)
        else:
            A = node.get_leaves_by_name(key)
            A[0].name += ": " + value

traverse_dict(load_dict, t)

print ("Original tree:")
print (t.get_ascii(show_internal=True))
ts = TreeStyle()
ts.show_leaf_name = False
def my_layout(node):
    F = TextFace(node.name, fsize=12, ftype="italic", bold=True)
    add_face_to_node(F, node, column=0, position="branch-top")
ts.scale = 20
ts.layout_fn = my_layout

def get_nodes(node):
    l = 0
    for n in node.traverse():
        l += 1
    return l

for n in t.traverse():
    l = get_nodes(n)
    width = int((math.log(l, 10))*4)
    style = NodeStyle()
    style["shape"] = "sphere"
    style["size"] = 8
    style["vt_line_color"] = "#000000"
    style["hz_line_color"] = "#000000"
    style["vt_line_type"] = 0
    style["hz_line_type"] = 0
    style["hz_line_width"] = width
    style["vt_line_width"] = width
    if(len(n) == 1):
        style["fgcolor"] = "#00ff00"
    n.set_style(style)
    # print(n)
    n.dist = len(n.name)*0.7

t.show(tree_style=ts)
t.render("mytree.png", tree_style=ts)
# t.show()