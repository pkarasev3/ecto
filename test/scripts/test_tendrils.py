#!/usr/bin/env python
import ecto
import ecto_test

def test_tendrils():
    t = ecto.Tendrils()
    t.declare("Hello","doc str",6)
    assert t.Hello == 6
    print t["Hello"].doc
    t.declare("x","a number", "str")
    assert len(t) == 2
    assert t["x"].val == "str"
    assert t.x == "str"
    #test the redeclare
    t.declare("Hello","new doc", "you")
    assert t.Hello == "you"
    assert t["Hello"].doc == "new doc"
    try:
        #read error
        t.nonexistant = 1
        assert False
    except RuntimeError,e:
        print "good:",e
    try:
        #index error
        print t["nonexistant"]
        assert False
    except KeyError,e:
        print "good:",e

    assert len(t.keys()) == 2
    assert len(t.values()) == 2
    
    print t
    #by value
    _x = t.x
    _x = 10
    assert t.x != 10
    x = t.x
    t.x = 11
    assert x != 11
    #by reference
    x = t["x"]
    t.x =13
    assert x.val == 13
    x.val = 15
    assert t.x == 15
    
if __name__ == '__main__':
    test_tendrils()
