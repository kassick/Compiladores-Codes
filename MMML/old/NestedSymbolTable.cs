using System.Collections.Generic;
using System;
using System.Linq;

namespace mimimil {

    public
    class SymbolEntry<T> {
        public T symbol;
        public int offset;
        public int size;

        public SymbolEntry(T symbol, int offset, int size) {
            this.symbol = symbol;
            this.offset = offset;

            Console.Clear();
            int i = this.
            this.size = size;
        }
    }

    public
    class NestedSymbolTable<T> : IEnumerable<SymbolEntry<T>> {
        /*-----------------------------------------------------------------------------
         * Function: NestedSymbolTable (ctor)
         * Description: Creates a new symbol table, child of parent, starting in the
         *              specified offset
         *---------------------------------------------------------------------------*/
        public NestedSymbolTable(NestedSymbolTable<T> parent, int offset) {
            this.Parent = parent;
            this.baseOffset = offset;
            this.NextOffset = offset;
            this.storage = new Dictionary<string, SymbolEntry<T>>();
            this.Nested = new List<NestedSymbolTable<T>>();
            if (parent != null) {
                this.entriesCount = parent.entriesCount;
                parent.Nested.Add(this);
            } else
                this.entriesCount = 0;
        }

        /*-----------------------------------------------------------------------------
         * Function: NestedSymbolTable (ctor)
         * Description: Created a new fist-level symbol table
         *---------------------------------------------------------------------------*/
        public NestedSymbolTable() : this(null, 0) {}

        /*-----------------------------------------------------------------------------
         * Function: NestedSymbolTable (ctor)
         * Description: Creates a new symbol table nested within parent
         *---------------------------------------------------------------------------*/
        public NestedSymbolTable(NestedSymbolTable<T> parent)
            : this(parent, parent == null? 0 : parent.NextOffset) {}

        /*-----------------------------------------------------------------------------
         * Function: Discard
         * Description: This table won't be used, remove it from parent list along with
         *              all it's children
         *---------------------------------------------------------------------------*/
        public void Discard() {
            if (Parent != null) {
                Parent.Nested.Remove(this);
            }

        }

        // Private Members
        private int
            baseOffset = 0,
            NextOffset = 0,
            size = 0,
            entriesCount = 0;

        private Dictionary<string, SymbolEntry<T> > storage;


        // Public Properties

        public NestedSymbolTable<T> Parent {get; set;}

        public List<NestedSymbolTable<T>> Nested {get; set;}

        public int BaseOffset {
            get { return baseOffset; }
        }

        public int Size {
            get { return size; }
        }

        public int Count {
            get { return entriesCount; }
        }

        /*-----------------------------------------------------------------------------
         * Function: NestedCount (property), get
         * Description: How many entries are there (total) in the whole symbol-table tree
         *              beginning at this one?
         *---------------------------------------------------------------------------*/
        public int NestedCount {
            get {
                if (Nested.Count == 0) return Count;
                else {
                    return Nested.Select(t => t.NestedCount).Max();
                }
            }
        }

        /*-----------------------------------------------------------------------------
         * Function: NestedSize (property), get
         * Description: What is the size needed to store the symbols in the symbol-table
         *              tree beginning at this one?
         *---------------------------------------------------------------------------*/
        public int NestedSize {
            get {
                if (Nested.Count == 0) return Size;
                else {
                    return Nested.Select(t => t.NestedSize).Max();
                }
            }
        }


        /*-----------------------------------------------------------------------------
         * Function: NestedSymbolTable<T>::store
         * Description: Stores a symbol on the symbol table. Default size of the
         *              symbol on memory is 1. If there is a name clash, discards
         *              the old symbol. This may leave holes in the memory, could be
         *              optimized
         *---------------------------------------------------------------------------*/
        public int store(string name, T symbol, int size=1) {
            int symbolOffset = this.NextOffset;
            if (!storage.ContainsKey(name))
                this.entriesCount++;
            this.size += size;
            this.NextOffset += size;

            storage[name] = new SymbolEntry<T>(symbol, symbolOffset, size);

            return symbolOffset;
        }

        /*-----------------------------------------------------------------------------
         * Function: lookup
         * Description: Looks up the simbol and gives the entry. Offset is absolute,
         *              from the root of the tree.
         *
         *              For relative offsets, do
         *                 var se = table.lookup(text);
         *                 int relOffset = se.offset - table.BaseOffset
         *---------------------------------------------------------------------------*/
        public SymbolEntry<T> lookup(string symbol, int maxLevel=int.MaxValue) {
            var cur_table = this;

            while (cur_table != null && maxLevel > 0) {
                if (cur_table.storage.ContainsKey(symbol)) {
                    return cur_table.storage[symbol];
                }

                cur_table = cur_table.Parent;

                maxLevel --;
            }

            return default(SymbolEntry<T>); // null
        }

        /*-----------------------------------------------------------------------------
         * Function: GetEnumerator
         * Description: Returns an enumerator over the sorted array of entries
         *              We fill entries with recursiveFillEntries, which will store
         *              in the array the entries of the parents and the current
         *---------------------------------------------------------------------------*/
        public IEnumerator<SymbolEntry<T>> GetEnumerator()
        {
            SymbolEntry<T>[] values = new SymbolEntry<T>[this.entriesCount];

            recursiveFillEntries(this, values);

            Array.Sort(values, (a, b) => b.offset.CompareTo(a.offset));

            foreach (var i in values) {
                yield return i;
            }
        }

        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            return this.GetEnumerator();
        }

        // Private methods

        /*-----------------------------------------------------------------------------
         * Function: RecurseFillEntries
         * Description: Stores in values all the entries in the current table and
         *              its parents as well
         *---------------------------------------------------------------------------*/
        private int recursiveFillEntries(NestedSymbolTable<T> current,
                                          SymbolEntry<T>[] values)
        {
            if (current == null)
                return 0;
            else {
                int offset = recursiveFillEntries(current.Parent, values);
                current.storage.Values.CopyTo(values, offset);
                return offset + current.storage.Count;
            }
        }




    }

}
