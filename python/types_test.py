from typing import List

def cheeseshop(kind, *arguments, **keywords):
    print(keywords)
    pass

def testAnnotation(l:int):
    print(len(l))

if __name__ == '__main__':
    d=dict(sape=4139, jack=4098, guido=4127)
    print(d)
    testAnnotation({"asdf":1200,"aaaaa":10000, "aaaaa1":10000})
    cheeseshop(1, 1,1,1, **d)
