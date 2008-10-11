#!/usr/bin/ruby

# Open Content Radio copyright (C) 2008 Jason Woofenden
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA


if (ARGV.length < 2) or (ARGV[0] == '--help') or (ARGV[0] == '-h')
	print "Usage: read_skin_coords.rb foo.svg tag1 [tag2 [...]]\n"
	print "foo.svg is inspected for all tags specified\n"
	print "if any of the tags is not found, returns an error code\n"
	print "if all tags are found, it outputs a .h file with the coordinates\n"
	exit 1
end

svg = ARGV.shift
tags = ARGV.clone

class String
	def starts_with(txt)
		return self.slice(0, txt.length) == txt
	end

	def esc_shell
		return "'#{self.gsub("'", "'\\\\''")}'"
	end
end

class Array
	def esc_shell
		ret = ''
		self.each { |arg|
			ret += ' ' + arg.esc_shell
		}
		return ret
	end
end

coords = Hash.new()

cmd = "inkscape --query-all #{svg.esc_shell}"
f = IO.popen(cmd)
f.each { |line|
	l = line.split(',')
	if(l.length == 5)
		key = l.shift
		#print "key: #{key}"
		if(tags.index(key))
			#print "   match!"
			coords[key] = l
		end
		#print "\n"
	end
}

if coords.length != tags.length
	# TODO say which one is missing
	print "ERROR: skin doesn't have all the tags it should. It must contain all these: #{tags.esc_shell}\n"
	exit(1)
end

out = File.new("skin_coords.h", "w")
out.print "// AUTO-GENERATED by read_skin_coords.rb from #{svg}\n"
coords.each { |tag,row|
	if(tag == 'background')
		x = y = 0;
		height = 150;
		width = 500;
	else
		x, y, w, h = row
		x = x.to_i
		y = y.to_i
		h = h.to_i
		w = w.to_i
		width  = (x + w).ceil
		x = x.floor
		width -= x
		height = (y + h).ceil
		y = y.floor
		height -= y

		# Either the coordinates are innacurate, or I'm rounding the wrong
		# way, also coordinates do not seem to include anti-aliasing pixels
		# or blurs.
		y -= 2;
		x -= 2;
		height += 7;
		width += 6;
	end
	out.print "\n"
	out.print "#define SKIN_#{tag.upcase}_LEFT #{x}\n"
	out.print "#define SKIN_#{tag.upcase}_TOP #{y}\n"
	out.print "#define SKIN_#{tag.upcase}_WIDTH #{width}\n"
	out.print "#define SKIN_#{tag.upcase}_HEIGHT #{height}\n"
	system("inkscape --export-id=#{tag.esc_shell} --export-id-only --export-area=#{x}:#{150-y-height}:#{x+width}:#{150-y} --export-width=#{width} --export-png='skin/#{tag}.png' #{svg}\n")
}
