#!/usr/bin/ruby

while gets()
  puts $_.gsub(/(?<=\W)(error:)(?=\W)/, "\e[1;38;5;9m\\1\e[0m").gsub(/(?<=\W)(warning:)(?=\W)/, "\e[1;38;5;13m\\1\e[0m")
end