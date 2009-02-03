require 'msf/core'


class Metasploit3 < Msf::Auxiliary

        include Msf::Exploit::COA
	
	def initialize
                super(
                        'Name'          => 'DECT Base Station Scanner',
                        'Version'       => '$revision$',
                        'Description'   => %q{
                                This module scans for DECT device base stations.
                        },
                        'Author'        =>
                                ['DK <privilegedmode@gmail.com>'],
                        'References'    =>
                                [
                                        ['Dedected', 'http://www.dedected.org'],
                                ],
                        'License'       => MSF_LICENSE
                )

                register_options(
                        [
				OptString.new('VERBOSE',[true,'Be verbose.',true])
                        ],
                        self.class
                )

        end

	base_stations = []

	def run
		print_status("Opening interface: #{datastore['INTERFACE']}")
		open_coa
		print_status("Using band: #{band}")
		print_status("Changing to fp scan mode.")
		fp_scan_mode
		print_status("Scanning..")

		while (true)
			data = poll

			if (data != nil)
				puts data
				parsed_data = parse_station(data)
				print_status("Found RFPI: #{parsed_data['rfpi']}")
			end

			next_channel

			if (datastore['VERBOSE'])
				print_status("Switching to channel: #{channel}")
			end
			sleep(1)
		end

		stop
		close_coa
	end
end
