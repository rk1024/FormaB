require_relative 'diag'

require 'set'

class ErrorableHash
  include Enumerable

  def initialize(hash, errors)
    @hash = hash.to_h
    @errors = Set.new(errors)
  end

  def hash; @hash.hash ^ @errors.hash end
  def eql?(other)
    @hash.eql?(other.instance_variable_get(:@hash)) &&
      @errors.eql?(other.instance_variable_get(:@errors))
  end
  alias :== :eql?

  def [](key)
    @hash.fetch(key) {|k| raise "Key #{k.inspect} represents an error" if @errors.include?(k) }
  end
  def []=(key, value)
    @hash[key] = value
    @errors.delete(key)
  end
  def make_error(key)
    @hash.delete(key)
    @errors << key
  end

  def clear
    @hash.clear
    @errors.clear
  end
  def clear_errors; @errors.clear end

  def delete(key, &block)
    @hash.delete(key) do |_|
      @errors.delete?(key) ? [key] : nil
    end
  end

  def each(&block) @hash.each(&block) end
  def each_key(&block)
    if block_given?
      @hash.each_key(&block)
      @errors.each(&block)
    else Enumerator.new{|y| each_key{|k| y << k } } end
  end
  def each_valid_key(&block) @hash.each_key(&block) end
  def each_error(&block) @errors.each(&block) end
  def each_value(&block) @hash.each_value(&block) end

  def empty?; @hash.empty? end

  def fetch(key, *args, &block) @hash.fetch(key, *args, &block) end
  def fetch_values(*keys, &block) @hash.fetch_values(*keys, &block) end

  def has_key?(key) @hash.has_key?(key) || @errors.include?(key) end
  def has_value?(value) @hash.has_value?(value) end
  alias include? has_key?
  def invert; ErrorableHash.new(@hash.invert, Set.new) end

  def is_error?(key) @errors.include?(key) end
  def isnt_error?(key) @hash.has_key?(key) end

  def keys
    @hash.keys + @errors
  end

  def length; @hash.length end

  def values; @hash.values end

  def table; @hash.clone end
  def errors; @errors.clone end

  def inspect; "#{self.class.name}{#{map{|k, v| "#{k.inspect}=>#{v.inspect}" }}}" end
  alias to_s inspect

  def diag_no_inspect
    return [:@errors] if @errors.empty?
    return [:@hash] if @hash.empty?
    []
  end
end

class LooseHash
  include Enumerable

  @@d = Diagnostics.new(pos: ["LooseHash"])

  def initialize(hash = nil)
    @hash = {}
    merge!(hash) if hash
  end

  def initialize_clone(other)
    @hash = {}
    merge!(other)
  end

  def [](key)
    ret = @hash[key]
    ret.clone if ret
  end

  def []=(key, val)
    case val
      when LooseHash
        case @hash[key]
          when LooseHash; @hash[key].merge!(val)
          when nil; @hash[key] = val.clone
          else raise "Cannot mix nested and non-nested values"
        end
      else self << [key, val]
    end

    self
  end

  def <<((key, val)) addn(key, val) end

  def addn(key, val, n: 1)
    raise "Use []= to add nested values" if val.is_a?(LooseHash)
    @hash[key] = {} unless @hash.has_key?(key)
    raise "Cannot mix nested and non-nested values" if @hash[key].is_a?(LooseHash)
    @hash[key][val] = 0 unless @hash[key].has_key?(val)
    @hash[key][val] += n
    self
  end

  def counted; @hash.clone end

  def each
    if block_given?
      @hash.each do |key, vals|
        case vals
          when LooseHash
            yield [key, vals]
          else
            vals.each do |val, n|
              n.times{ yield [key, val] }
            end
        end
      end
    else Enumerator.new{|y| each{|p| y << p } } end
  end
  def each_key(&block) @hash.each_key(&block) end
  def each_value
    if block_given?
      @hash.each_value do |vals|
        vals.each_key{|k| yield k }
      end
    else Enumerator.new{|y| each_value{|v| y << v } } end
  end

  def empty?; @hash.empty? end

  def fetch(key, default = nil, &block)
    if default
      @hash.fetch(key, default, &block)
    else
      @hash.fetch(key, &block)
    end
  end

  def has_key?(key) @hash.has_key?(key) end
  def has_value?(value) @hash.any?{|_, v| v.has_key?(value) } end

  def invert
    ret = LooseHash.new
    each{|k, v| ret[v] = k }
    ret
  end

  def invert_grouped
    ret = LooseHash.new
    @hash.each{|k, v| ret[v.each_key.to_a] = k }
    ret
  end

  def length; @hash.length end

  def rehash; @hash.rehash end

  def flatten(&block)
    errors = Set.new
    hash = @hash.select do |key, vals|
      next false if vals.empty?
      next true if vals.is_a?(LooseHash)

      if vals.length > 1 || vals.first[1] > 1
        if !block_given?
        elsif block.arity == 3
          yield key, vals, vals.reduce(0) {|curr2, (_, n)| curr2 + n }
        else
          yield key, vals
        end
        errors << key
        next false
      end
      true
    end.map{|k, v| [k, case v
        when LooseHash; v
        else v.first[0]
      end] }.to_h

    ErrorableHash.new(hash, errors)
  end

  def dedup(&block)
    hash = @hash.map do |key, vals|
      [
        key,
        case vals
          when LooseHash; vals
          else vals.map{|(v, _)| v }
        end,
      ]
    end.select{|(_, v)| !v.empty? }.to_h

    ErrorableHash.new(hash, Set.new)
  end

  def merge(other)
    ret = self.clone
    ret.merge!(other)
    ret
  end

  def merge!(other)
    case other
      when nil
      when LooseHash
        other.counted.each do |key, vals|
          case vals
            when LooseHash
              self[key] = vals.clone
            else
              vals.each{|v, n| addn(key, v, n: n) }
          end
        end
      else
        other.each{|k, v| addn(k, v, n: 1) }
    end
  end

  def inspect; "#{self.class.name}{#{map{|k, v| "#{k.inspect}=>#{v.inspect}"}.join(", ")}}" end
  alias to_s inspect

  def diag_no_inspect; [] end
end