#!/usr/bin/env rvm 1.9.3 do ruby
require 'socket'
require 'thread'

begin
	require 'ncurses'
	HAS_NCURSES = true
rescue LoadError => err
	HAS_NCURSES = false
	require 'readline'
	puts "running without ncurses"
end

include Socket::Constants

class DebugClient
	def initialize
		@stop = false
		@connected = false
		@out_queue = []
		@sem = Mutex.new
		if HAS_NCURSES
			Ncurses.initscr
			Ncurses.cbreak
			Ncurses.noecho
			# recognize KEY_ENTER, KEY_BACKSPACE  etc
			Ncurses.keypad(Ncurses.stdscr, true)
			@input_win = Ncurses.newwin(0, 0, Ncurses.stdscr.getmaxy - 2, 0)
			@output_win = Ncurses.newwin(Ncurses.stdscr.getmaxy - 2, Ncurses.stdscr.getmaxx, 0, 0)
			Ncurses.scrollok(@output_win, true)
			@history = []
			@history_idx = 0
		end
	end

	def run_client
		return if @connected
		@socket_thread = Thread.new do
			# client_socket = Socket.new(AF_INET, SOCK_STREAM, 0)
			# sockaddr = Socket.sockaddr_in(1337, '127.0.0.1')
			while not @connected
				begin
					client_socket = TCPSocket.new 'localhost', 1337
					@connected = true
					puts "Connected to server"
				rescue => err
					puts "trying again (#{err})"
					sleep 1.0
				end
			end
			while not @stop
				rs, ws = IO.select([client_socket], [client_socket])
				income = nil
				if s = rs[0]
					income = s.gets
					income = nil if income == "<"
				end
				if s = ws[0]
					out = nil
					@sem.synchronize {
						out = @out_queue.shift
					}
					if out
						client_socket.puts out
					end
				end
				if income
					puts income
				end
				sleep 0.2
			end
		end
	end

	def show_help
		puts <<-EOS
Available commands:

break <file.js>:<line_number> # sets a breakpoint in the specified file and
                                line number
eval <valid js>               # will eval the argument in the current stack
                                frame
scripts                       # lists all the loaded scripts
cont                          # continues execution
line                          # prints the current line
bt                            # prints the current back trace
step                          # continues execution until next line
help                          # prints this help message

Up & down arrows traverse history
		EOS
	end

	def begin
		run_client
		while buf = read_line
			if buf == "exit"
				@stop = true
				@socket_thread.kill if @socket_thread
				@socket_thread.join if @socket_thread
				if HAS_NCURSES
					Ncurses.endwin
				end
				exit(0)
			elsif buf == "help"
				show_help
			elsif !buf.empty?
				@sem.synchronize {
					@out_queue << buf.strip
				}
			end
		end
	end # begin

private
	def puts(str)
		if HAS_NCURSES
			@output_win.addstr(str + "\n")
			@output_win.refresh
			@input_win.move(2, 0)
			@input_win.refresh
		else
			Kernel.puts str
		end
	end

	def read_line
		if HAS_NCURSES
			return read_line_ncurses("> ", @input_win)
		else
			return Readline.read_line("> ", true)
		end
	end

	# read_line returns an array
	# [string, last_cursor_position_in_string, keycode_of_terminating_enter_key].
	# Complete the "when" clauses before including in your app!
	def read_line_ncurses(prompt = "> ", window = Ncurses.stdscr, max_len = (window.getmaxx - 1), cursor_pos = 0)
		x = prompt.length
		y = 0
		window.mvaddstr(y, 0, prompt)
		string = ""
		loop do
			window.mvaddstr(y,x,string)
			window.move(y,x+cursor_pos)
			ch = window.getch
			case ch
			when Ncurses::KEY_LEFT
				cursor_pos = [0, cursor_pos-1].max
			when Ncurses::KEY_RIGHT
				cursor_pos = [cursor_pos+1, string.size].min
			when Ncurses::KEY_UP
				if @history_idx > 0
					# clear old line
					window.mvaddstr(y, x, " " * string.size)
					# add history line
					@history_idx -= 1
					string = @history[@history_idx]
					cursor_pos = string.length
				else
					Ncurses.beep
				end
			when Ncurses::KEY_DOWN
				if @history_idx < @history.size - 1
					# clear old line
					window.mvaddstr(y, x, " " * string.size)
					# add history line
					@history_idx += 1
					string = @history[@history_idx]
					cursor_pos = string.length
				end
			when Ncurses::KEY_ENTER, 10
				unless string.empty?
					@history[@history_idx] = string
					@history_idx += 1
					window.mvaddstr(y, x, " " * string.length)
				end
				return string
			when Ncurses::KEY_BACKSPACE, 127
				string = string[0...([0, cursor_pos-1].max)] + string[cursor_pos..-1]
				cursor_pos = [0, cursor_pos-1].max
				window.mvaddstr(y, x+string.length, " ")
			# when etc...
			when " ".getbyte(0)..255 # remaining printables
				if (cursor_pos < max_len)
					string[cursor_pos,0] = ch.chr
					cursor_pos += 1
				else
					Ncurses.beep
				end
			else
				Ncurses.beep
			end
		end # loop
	end
end

if !HAS_NCURSES
	LIST = %w(
		break bt continue
		eval exit help step
	).sort

	comp = proc { |s| LIST.grep(/^#{Regexp.escape(s)}/) }
	Readline.completion_proc = comp
end

DebugClient.new.begin
