#!/usr/bin/env ruby
require 'readline'
require 'socket'
require 'thread'

include Socket::Constants

class DebugClient
	def initialize
		@stop = false
		@connected = false
		@out_queue = []
		@sem = Mutex.new
	end

	def run_client
		return if @connected
		@socket_thread = Thread.new do
			client_socket = Socket.new(AF_INET, SOCK_STREAM, 0)
			sockaddr = Socket.sockaddr_in(1337, '127.0.0.1')
			begin
				client_socket.connect_nonblock(sockaddr)
			rescue => err
			end
			@connected = true
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

break <file.js>:<line_number> # currently not working ok
eval <valid js>               # will eval the argument in the current stack frame
line                          # prints the current line
bt                            # prints the current back trace
step                          # continues execution until next line
help                          # prints this help message

Up arrow traverses history
		EOS
	end

	def begin
		puts "Press enter to connect to the server (once it's ready)"
		while buf = Readline.readline("> ", true)
			if buf == "" and not @connected
				run_client
				next
			end
			if buf == "exit"
				@stop = true
				@socket_thread.kill
				@socket_thread.join
				exit(0)
			elsif buf == "help"
				show_help
			else
				@sem.synchronize {
					@out_queue << buf.strip
				}
			end
		end
	end # begin
end

LIST = %w(
	break bt continue
	eval exit help step
).sort

comp = proc { |s| LIST.grep(/^#{Regexp.escape(s)}/) }
Readline.completion_proc = comp

DebugClient.new.begin
