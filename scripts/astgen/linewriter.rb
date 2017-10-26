##########################################################################
# 
# FormaB - the bootstrap Forma compiler (linewriter.rb)
# Copyright (C) 2017 Ryan Schroeder, Colin Unger
# 
# FormaB is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
# 
# FormaB is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
# 
# You should have received a copy of the GNU Affero General Public License
# along with FormaB.  If not, see <https://www.gnu.org/licenses/>.
# 
##########################################################################

class LineWriter
  class BaseFormatter
    def initialize(arr)
      @arr = arr
      @sep = 0
    end

    def trim
      @sep.times { @arr.pop }
      @sep = 0
    end

    def <<(val)
      str = val.to_s
      str.rstrip!
      relay str, is_first: @arr.none?
    end

    def relay(str, is_first:)
      if str.strip.empty?
        @sep += 1
      else
        @sep = 0
      end

      @arr << str

      self
    end

    def sep(spacing, and_merge: true, unless_first: true)
      return self if unless_first and @arr.none?

      spacing -= @sep if and_merge

      spacing.times { @arr << "" }

      @sep += spacing

      self
    end
  end

  class Formatter
    attr_reader :spacing, :indent

    def initialize(with_spacing: 0, with_indent: "", child_of: nil)
      @spacing = with_spacing
      @indent = with_indent
      @parent = child_of
      @first = true
    end

    def <<(val)
      sep @spacing

      str = val.to_s
      str.rstrip!

      relay str, is_first: @first
    end

    def relay(str, is_first:)
      sep @spacing if is_first

      str.prepend(@indent) if !str.empty?

      @first = false if @first

      @parent.relay str, is_first: is_first

      self
    end

    def sep(spacing = 1, and_merge: true, unless_first: true)
      return self if unless_first and @first

      @parent.sep spacing, and_merge: and_merge, unless_first: unless_first

      self
    end

    def group
      sep @spacing

      @first = true
    end

    def close; @parent = nil end
  end

  def initialize()
    @lines = []
    @base_fmt = BaseFormatter.new(@lines)
    @stack = [@base_fmt]
  end

  def self.lines(with_spacing: 0, with_indent: "")
    l = LineWriter.new()

    l.fmt with_spacing: with_spacing, with_indent: with_indent do
      yield l
    end

    l.to_s
  end

  def curr; @stack.last end

  def peek; @lines.last end

  def fmt(with_spacing: 0, with_indent: "")
    @stack.push Formatter.new(
      with_spacing: with_spacing,
      with_indent: with_indent,
      child_of: curr
    )

    yield

    curr.close
    @stack.pop
  end

  def group; curr.group; self end

  def trim; @base_fmt.trim; self end

  def <<(val) curr << val; self end

  def sep(spacing = 1, and_merge: true, unless_first: true)
    curr.sep spacing, and_merge: and_merge, unless_first: unless_first
    self
  end

  def to_s; "#{@lines.join("\n")}" end
end
