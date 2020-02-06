#include <iostream>
#include <thread>
#include <chrono>
#include <numeric>

#include <libtorrent/session.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/magnet_uri.hpp>

// Use them to construct priorities vector from the ground
//#include <boost/assign/list_of.hpp>
//#include <boost/assign/std/vector.hpp>

void print_bitmask(lt::torrent_handle handle);
bool need_prios(lt::torrent_handle handle);
std::vector<lt::download_priority_t> give_prios(lt::torrent_handle handle);

int main(int argc, char* argv[]) try
{
	if (argc != 2) {
		std::cerr << "usage: " << argv[0] << " <magnet-url>" << std::endl;
		return 1;
	}
	int got_data = 0;
	int needs_prios = 1;
	lt::settings_pack p;
	p.set_int(lt::settings_pack::alert_mask, lt::alert::status_notification
			| lt::alert::error_notification);
	lt::session ses(p);

	lt::add_torrent_params atp = lt::parse_magnet_uri(argv[1]);
	atp.save_path = "."; // save in current dir
	lt::torrent_handle h = ses.add_torrent(std::move(atp));

	//lt::session s;
	//lt::add_torrent_params p;
	//p.save_path = "./";
	//p.ti = std::make_shared<lt::torrent_info>(argv[1]);
	//lt::torrent_handle h = s.add_torrent(p);

	//h.lt::torrent_handle::set_sequential_download(true);
	//std::cout << h.lt::torrent_handle::is_sequential_download() << std::endl;

	for (;;) {
		std::vector<lt::alert*> alerts;
		ses.pop_alerts(&alerts);

		for (lt::alert const* a : alerts) {
			//std::cout << a->message() << std::endl;
			// if we receive the finished alert or an error, we're done
			if (lt::alert_cast<lt::metadata_received_alert>(a)) {
				std::cout << "got metadata" << std::endl;
				sleep(10);
				got_data = 1;
				//std::cout << h.status().pieces[0] << std::endl;
				//std::cout << h.status().num_pieces << std::endl;
				//std::cout << h.status().list_seeds << std::endl;
				//std::cout << std::to_string(h.get_piece_priorities()[1]) << std::endl;
				//h.piece_priority(lt::piece_index_t(1), lt::download_priority_t(7));
				//std::cout << std::to_string(h.piece_priority(1)) << std::endl;
				//goto done;
			}
			if (lt::alert_cast<lt::torrent_finished_alert>(a)) {
				goto done;
			}
			if (lt::alert_cast<lt::torrent_error_alert>(a)) {
				goto done;
			}
		}
		if (got_data == 1) {
			if (needs_prios) {
				h.prioritize_pieces(give_prios(h));
				needs_prios = 0;
			}
			//std::cout << h.status().pieces[0] << std::endl;
			//std::cout << h.status().pieces[1] << std::endl;
			//std::cout << h.status().pieces[2] << std::endl;
			//std::cout << h.status().num_pieces << std::endl;
			print_bitmask(h);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	}
done:
	std::cout << "done, shutting down" << std::endl;


	// wait for the user to end
	//char a;
	//int ret = std::scanf("%c\n", &a);
	//(void)ret; // ignore
	return 0;
}
catch (std::exception const& e) {
	std::cerr << "ERROR: " << e.what() << "\n";
}

void print_bitmask(lt::torrent_handle handle) {
	int i = 0;
	int size = handle.status().pieces.size();
	while (i < size) {
		std::cout << handle.status().pieces[i];
		i++;
	}
	std::cout << std::endl;
	std::cout << "-------------------";
	std::cout << handle.status().num_pieces;
	std::cout << "-------------------" << std::endl;
}

bool need_prios(lt::torrent_handle handle) {
	int size = handle.status().pieces.size();
	std::vector<lt::download_priority_t> prios = handle.get_piece_priorities();
	int prioritized = std::count(prios.begin(), prios.end(), 7);
	int prioritized_got = 0;
	int i = 0;
	while (i < prioritized) {
		prioritized_got += (int)handle.status().pieces[i];
	}
	if (prioritized == 0 || (float)prioritized_got / (float)prioritized > 0.9) {
		return true;
	}
	return false;
}

std::vector<lt::download_priority_t> give_prios(lt::torrent_handle handle) {
	int i = 0;
	int size = handle.status().pieces.size();
	std::vector<lt::download_priority_t> prios = handle.get_piece_priorities();

	std::vector<lt::download_priority_t>::iterator it = std::find(prios.begin(), prios.end(), 4);
	int first_default = std::distance(prios.begin(), it);
	int offset = first_default; // ? Do better?

	while (i + offset < size * 0.02) {
		prios[i] = 7;
		i++;
	}

	return prios;
}
