using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;

namespace Cats {
  public struct Info {
    public int Length { get; }

    public string Value { get; }

    public string Name { get; }

    public string Category { get; }

    public string CanonCombClass { get; }

    public string BidiClass { get; }

    public string DecompInfo { get; }

    public string NumericInfo { get; }

    public string BidiMirrored { get; }

    public string UpperMapping { get; }

    public string LowerMapping { get; }

    public string TitleMapping { get; }

    public Info(int len, string value, string name, string cat, string ccClass, string bdClass,
                string decomp, string numeric, string mirrored, string upper,
                string lower, string title) {
      Length = len;
      Value = value;
      Name = name;
      Category = cat;
      CanonCombClass = ccClass;
      BidiClass = bdClass;
      DecompInfo = decomp;
      NumericInfo = numeric;
      BidiMirrored = mirrored;
      UpperMapping = upper;
      LowerMapping = lower;
      TitleMapping = title;
    }
  }

  public static class Program {
    public static void Main(params string[] args) {
      var infos = new Dictionary<string, List<Info>>();

      using (var fs = new FileStream("src/UnicodeData.txt", FileMode.Open, FileAccess.Read))
      using (var reader = new StreamReader(fs)) {
        while (true) {
          string str = reader.ReadLine()?.Trim();

          if (str == null) break;

          string[] parts = str.Split(';');

          uint code = uint.Parse(parts[0], NumberStyles.HexNumber);

          var bytes = new List<byte>();

          if (code <= 0x7F) bytes.Add((byte) code);
          else if (code <= 0x7FF) {
            bytes.Add((byte) (0xC0 | (code >> 6)));
            bytes.Add((byte) (0x80 | (code & 0x3F)));
          }
          else if (code <= 0xFFFF) {
            bytes.Add((byte) (0xE0 | (code >> 12)));
            bytes.Add((byte) (0x80 | ((code >> 6) & 0x3F)));
            bytes.Add((byte) (0x80 | (code & 0x3F)));
          }
          else if (code <= 0x10FFFF) {
            bytes.Add((byte) (0xF0 | (code >> 18)));
            bytes.Add((byte) (0x80 | ((code >> 12) & 0x3F)));
            bytes.Add((byte) (0x80 | ((code >> 6) & 0x3F)));
            bytes.Add((byte) (0x80 | (code & 0x3F)));
          }
          else throw new InvalidOperationException($"Invalid code point {code:X8}.");

          Info info = new Info(bytes.Count, Encoding.UTF8.GetString(bytes.ToArray()),
                               parts[1], parts[2], parts[3], parts[4],
                               parts[5], parts[6], parts[7], parts[10], parts[11], parts[12]);

          List<Info> list;
          if (!infos.TryGetValue(parts[2], out list))
            infos.Add(parts[2], list = new List<Info>());

          list.Add(info);
        }
      }

      Console.OutputEncoding = Encoding.UTF8;

      var keys = new SortedSet<string>(infos.Keys);

      foreach (var pair in from key in keys
                           select new { Key = key, Value = infos[key] }) {
        Console.WriteLine($"-----\n{pair.Key}\n-----\n");

        foreach (var info in pair.Value) {
          Console.WriteLine($"({info.Length}) \u200D{info.Value}\u200D : {info.Name} ({info.BidiClass}; {info.DecompInfo}; {info.NumericInfo})");
        }
      }
    }
  }
}