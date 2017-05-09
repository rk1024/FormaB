class Diagnostics
  class DiagnosticError < RuntimeError; end

  def initialize(pos:[], on_error:, quiet:, verbose:)
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

  def good(lvl, str)
    trace(lvl, str) unless @quiet.call
    str
  end

  def verb(lvl, str)
    trace(lvl, str) if @verbose.call
    str
  end

  def bad(lvl, str)
    trace(lvl, str)
    @on_error.call
    str
  end

  def bad_r(lvl, str)
    DiagnosticError.new bad(lvl, str)
  end

  def debug(str) good("debug", str) end
  def info(str) good("info", str) end
  def warn(str) good("warning", str) end
  def debug_v(str) verb("debug", str) end
  def info_v(str) verb("info", str) end
  def warn_v(str) verb("warning", str) end
  def error(str) bad("error", str) end
  def fatal(str) bad("fatal", str) end
  def error_r(str) bad_r("error", str) end
  def fatal_r(str) bad_r("fatal", str) end

  def p(obj) debug(obj.inspect) end
  def p_v(obj) debug_v(obj.inspect) end
end