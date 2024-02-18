python

ludo = gdb.printing.RegexpCollectionPrettyPrinter("ludo")

class ArrayIterator:
    def __init__ (self, begin, end):
        self.begin = begin
        self.end = end
        self.index = -1
        self.current = begin - 1

    def __iter__ (self):
        return self

    def __next__ (self):
        self.index = self.index + 1
        self.current = self.current + 1

        if self.current == self.end:
            raise StopIteration

        return ('[%d]' % self.index, self.current.dereference())

    def next (self):
        return self.__next__()

class ArrayBufferPrinter:
    "Print an array_buffer"

    def __init__ (self, val):
        self.val = val

    def children (self):
        element_type = self.val.type.template_argument(0)
        start = self.val['data'].cast(element_type.pointer())
        return ArrayIterator(start, start + self.val['array_size'])

    def to_string (self):
        element_type = self.val.type.template_argument(0)
        capacity = self.val['size'] / element_type.sizeof
        return '%s of length %d, capacity %d' % (self.val.type, self.val['array_size'], capacity)

    def display_hint (self):
        return 'array'

ludo.add_printer('array_buffer', '^ludo::(array|partitioned)_buffer', ArrayBufferPrinter)

gdb.printing.register_pretty_printer(gdb.current_objfile(), ludo)
end
