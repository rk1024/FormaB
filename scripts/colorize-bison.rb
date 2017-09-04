#!/usr/bin/ruby

while gets()
  colors = {
    info: 8,
    note: 8,
    error: 9,
    warning: 13,
  }.map{|k, v| ["#{k}:", "\e[38;5;#{v}m#{k}:\e[0;1m"] }.to_h

  puts($_
    .gsub(/^.*:\s*(?:error|warning)\s*:.*$/i, "\e[1m\\0\e[0m")
    .gsub(/(?<=:|\s)((?:error|warning):)/i, colors)
    .gsub(/^([\s\^]+)$/i, "\e[38;5;10m\\0\e[0m"))
end