require_relative 'linewriter'

require 'set'

class Diagnostics
  class DiagnosticError < RuntimeError; end

  def initialize(pos: [], on_error: nil, quiet: nil, verbose: nil)
    @pos = pos
    @on_error = on_error
    @quiet = quiet
    @verbose = verbose
  end

  def fork
    Diagnostics.new(pos: @pos.clone, on_error: @on_error, quiet: @quiet, verbose: @verbose)
  end

  def push(str) @pos.push(str); self end
  def pop; @pos.pop(); self end

  def pos(str)
    push(str)

    begin
      return yield
    ensure
      pop()
    end
  end

  def trace(lvl, str)
    $stderr << "[#{@pos.join(" :: ")}] #{lvl}: #{str}\n"
    str
  end

  def ctrace(lvl, fmt, str)
    $stderr << "\e[1m[#{@pos.join(" :: ")}]\e[0m \e[#{fmt}m#{lvl}:\e[0m #{str}\n"
    str
  end

  def good(lvl, fmt, str)
    ctrace(lvl, fmt, str) unless @quiet && @quiet.call
    str
  end

  def verb(lvl, fmt, str)
    ctrace(lvl, fmt, str) if @verbose && @verbose.call
    str
  end

  def bad(lvl, fmt, str)
    ctrace(lvl, fmt, str)
    @on_error.call if @on_error
    str
  end

  def bad_r(lvl, fmt, str)
    DiagnosticError.new bad(lvl, fmt, str)
  end

  def hl_uname(name, last = true)
    name = name.to_s unless name.is_a?(String)
    if name =~ /^(@|@@|\$)(.*)$/
      "\e[38;5;2m#{$1}\e[38;5;4m#{$2}"
    else
      "\e[38;5;#{last ? "4" : "2"}m#{name}"
    end
  end

  def hl_qname(name)
    name = name.to_s unless name.is_a?(String)
    name = name.split(/(::|#|\.)/)

    return "" if name.empty?

    name.each_with_index.map do |part, idx|
      next "" if part.empty?
      idx % 2 == 0 ? hl_uname(part, idx == name.length - 1) : "\e[0m#{part}"
    end.join
  end

  private def hl_fold(obj)
    remain = 32

    catch :break do
      count = lambda do |obj|
        throw :break unless remain > 0

        case obj
          when Array
            return false if obj.length <= 1

            remain -= 2 * [1, obj.length].max
            obj.each{|e| count.call(e) }
          when Hash
            return false if obj.length <= 1

            remain -= 2 + (obj.empty? ? 0 : 2) + 4 * [0, obj.length - 2].max
            obj.each{|k, v| count.call(k); count.call(v) }
          when Set
            return false if obj.length <= 1

            remain -= 1 + obj.class.name.length + 2 * [1, obj.length].max
            obj.each{|e| count.call(e) }
          else
            has_no_inspect = obj.class.method_defined?(:diag_no_inspect)
            if obj.method(:inspect).owner == Kernel || has_no_inspect
              no_inspect = has_no_inspect ? Set.new(obj.diag_no_inspect) : Set.new

              remain -= 23 + obj.class.name.length

              first = true
              obj.instance_variables.select do |el|
                next false if no_inspect.include?(el)

                remain -= if first
                  first = false
                  3
                else 5 end

                true
              end.each{|e| count.call(obj.instance_variable_get(e)) }
            else remain -= obj.inspect.length end
        end
      end

      count.call(obj)
    end

    return remain <= 0
  end

  def hl(obj, long: false)
    lsp = long ? " " : ""

    case obj
      when Array
        items = obj.map{|e| hl(e, long: long) }

        if long && hl_fold(obj)
          LineWriter.lines do |l|
            l << "\e[0m["
            l.fmt with_indent: "  " do
              items.each_with_index do |el, idx|
                l.peek << "\e[0m," unless idx == 0
                el.to_s.split("\n").each{|e| l << e }
              end
            end
            l << "\e[0m]"
          end
        else
          "[#{items.join("\e[0m, ")}]"
        end
      when Hash, Set
        items = if obj.is_a?(Set)
          obj.map{|e| hl(e, long: long) }
        else
          obj.select{|k, _| k.is_a?(Symbol) }.map do |key, val|
            "\e[38;5;3m#{key.to_s}\e[0m: #{hl(val, long: long)}"
          end.concat(obj.select{|k, _| !k.is_a?(Symbol) }.map do |key, val|
            "#{hl(key, long: long)}\e[0m#{lsp}=>#{lsp}#{hl(val, long: long)}"
          end)
        end

        s = if long && hl_fold(obj)
          LineWriter.lines do |l|
            l << "\e[0m{"
            l.fmt with_indent: "  " do
              items.each_with_index do |el, idx|
                l.peek << "\e[0m," unless idx == 0
                el.split("\n").each{|e| l << e }
              end
            end
            l << "\e[0m}"
          end
        else
          "\e[0m{#{items.join("\e[0m, ")}\e[0m}"
        end

        if obj.is_a?(Hash)
          "\e[0m#{s}"
        else
          "\e[38;5;9m#\e[0m<#{hl_qname(obj.class.name)}\e[0m: #{s}>"
        end
      else
        has_no_inspect = obj.class.method_defined?(:diag_no_inspect)
        if obj.method(:inspect).owner == Kernel || has_no_inspect
          no_inspect = has_no_inspect ? Set.new(obj.diag_no_inspect) : Set.new

          prefix = "\e[38;5;9m#\e[0m<#{hl_qname(obj.class.name)}\e[0m:\e[38;5;5m0x#{obj.object_id.to_s(16)}"

          items = obj.instance_variables.select{|e| !no_inspect.include?(e) }.map do |e|
            hl_uname(e) << "\e[0m#{lsp}=#{lsp}#{hl(obj.instance_variable_get(e), long: long)}"
          end

          if long && hl_fold(obj)
            LineWriter.lines do |l|
              l << prefix
              l.fmt with_indent: "  " do
                items.each_with_index do |el, idx|
                  l.peek << "\e[0m," unless idx == 0
                  el.split("\n").each{|e| l << e }
                end
              end
              l << "\e[0m>"
            end
          else
            "#{prefix} #{items.join("\e[0m,  ")}\e[0m>"
          end
        else
          "\e[38;5;#{case obj
            when Symbol, true, false, nil; "3"
            when String; "6"
            when Numeric; "5"
            else "13"
          end}m#{obj.inspect}\e[0m"
        end
    end
  end

  def debug(str) good("debug", "38;5;14;1", str) end
  def info(str) good("info", "38;5;14;1", str) end
  def warn(str) good("warning", "38;5;13;1", str) end
  def debug_v(str) verb("debug", "38;5;14;1", str) end
  def info_v(str) verb("info", "38;5;14;1", str) end
  def warn_v(str) verb("warning", "38;5;13;1", str) end
  def error(str) bad("error", "38;5;1;1", str) end
  def fatal(str) bad("fatal", "38;5;1;1", str) end
  def error_r(str) bad_r("error", "38;5;1;1", str) end
  def fatal_r(str) bad_r("fatal", "38;5;1;1", str) end

  def p(obj, long: false) debug(hl(obj, long: long)) end
  def p_v(obj, long: false) debug_v(hl(obj, long: long)) end
end