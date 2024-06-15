#
# test07.py
#
# a nuPython program of binary expressions
#
print("TEST CASE: test07.py")
print()

# random comment

x = 100        # 100
y = x - 140    # -40

print("x is:")
print(x)
print('y is:')
print(y)

z = y * y      # 1600
print("and now for z:")
print(z)

test1 = x + y    # 60
x = 1
test2 = x + y    # -39
y = 123
test3 = x * y    # 123
test4 = z / 3    # 533
test5 = z % 11   # 5
x = 4
test6 = test5 ** x   # 625

print("test variables:")
print(test1)
print(test2)
print(test3)
print(test4)
print(test5)
print(test6)

# another random comment

print()
print("DONE")