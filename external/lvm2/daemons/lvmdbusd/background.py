# Copyright (C) 2015-2016 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

import threading
import subprocess
from . import cfg
import time
from .cmdhandler import options_to_cli_args
import dbus
from .utils import pv_range_append, pv_dest_ranges, log_error, log_debug
import traceback

_rlock = threading.RLock()
_thread_list = list()


def pv_move_lv_cmd(move_options, lv_full_name,
					pv_source, pv_source_range, pv_dest_range_list):
	cmd = ['pvmove', '-i', '1']
	cmd.extend(options_to_cli_args(move_options))

	if lv_full_name:
		cmd.extend(['-n', lv_full_name])

	pv_range_append(cmd, pv_source, *pv_source_range)
	pv_dest_ranges(cmd, pv_dest_range_list)

	return cmd


def lv_merge_cmd(merge_options, lv_full_name):
	cmd = ['lvconvert', '--merge', '-i', '1']
	cmd.extend(options_to_cli_args(merge_options))
	cmd.append(lv_full_name)
	return cmd


def _move_merge(interface_name, cmd, job_state):
	add(cmd, job_state)

	done = job_state.Wait(-1)
	if not done:
		ec, err_msg = job_state.GetError
		raise dbus.exceptions.DBusException(
			interface_name,
			'Exit code %s, stderr = %s' % (str(ec), err_msg))

	cfg.load()
	return '/'


def move(interface_name, lv_name, pv_src_obj, pv_source_range,
			pv_dests_and_ranges, move_options, job_state):
	"""
	Common code for the pvmove handling.
	:param interface_name:  What dbus interface we are providing for
	:param lv_name:     Optional (None or name of LV to move)
	:param pv_src_obj:  dbus object patch for source PV
	:param pv_source_range: (0,0 to ignore, else start, end segments)
	:param pv_dests_and_ranges: Array of PV object paths and start/end segs
	:param move_options: Hash with optional arguments
	:param job_state: Used to convey information about jobs between processes
	:return: '/' When complete, the empty object path
	"""
	pv_dests = []
	pv_src = cfg.om.get_object_by_path(pv_src_obj)
	if pv_src:

		# Check to see if we are handling a move to a specific
		# destination(s)
		if len(pv_dests_and_ranges):
			for pr in pv_dests_and_ranges:
				pv_dbus_obj = cfg.om.get_object_by_path(pr[0])
				if not pv_dbus_obj:
					raise dbus.exceptions.DBusException(
						interface_name,
						'PV Destination (%s) not found' % pr[0])

				pv_dests.append((pv_dbus_obj.lvm_id, pr[1], pr[2]))

		# Generate the command line for this command, but don't
		# execute it.
		cmd = pv_move_lv_cmd(move_options,
								lv_name,
								pv_src.lvm_id,
								pv_source_range,
								pv_dests)

		return _move_merge(interface_name, cmd, job_state)
	else:
		raise dbus.exceptions.DBusException(
			interface_name, 'pv_src_obj (%s) not found' % pv_src_obj)


def merge(interface_name, lv_uuid, lv_name, merge_options, job_state):
	# Make sure we have a dbus object representing it
	dbo = cfg.om.get_object_by_uuid_lvm_id(lv_uuid, lv_name)
	if dbo:
		cmd = lv_merge_cmd(merge_options, dbo.lvm_id)
		return _move_merge(interface_name, cmd, job_state)
	else:
		raise dbus.exceptions.DBusException(
			interface_name,
			'LV with uuid %s and name %s not present!' % (lv_uuid, lv_name))


def background_reaper():
	while cfg.run.value != 0:
		with _rlock:
			num_threads = len(_thread_list) - 1
			if num_threads >= 0:
				for i in range(num_threads, -1, -1):
					_thread_list[i].join(0)
					if not _thread_list[i].is_alive():
						log_debug("Removing thread: %s" % _thread_list[i].name)
						_thread_list.pop(i)

		time.sleep(3)


def background_execute(command, background_job):

	# Wrap this whole operation in an exception handler, otherwise if we
	# hit a code bug we will silently exit this thread without anyone being
	# the wiser.
	try:
		process = subprocess.Popen(command, stdout=subprocess.PIPE,
									stderr=subprocess.PIPE, close_fds=True)
		lines_iterator = iter(process.stdout.readline, b"")
		for line in lines_iterator:
			line_str = line.decode("utf-8")

			# Check to see if the line has the correct number of separators
			try:
				if line_str.count(':') == 2:
					(device, ignore, percentage) = line_str.split(':')
					background_job.Percent = \
						round(float(percentage.strip()[:-1]), 1)
			except ValueError:
				log_error("Trying to parse percentage which failed for %s" %
					line_str)

		out = process.communicate()

		if process.returncode == 0:
			background_job.Percent = 100

		background_job.set_result(process.returncode, out[1])

	except Exception:
		# In the unlikely event that we blow up, we need to unblock caller which
		# is waiting on an answer.
		st = traceback.format_exc()
		error = "Exception in background thread: \n%s" % st
		log_error(error)
		background_job.set_result(1, error)


def add(command, reporting_job):
	# Create the thread, get it running and then add it to the list
	t = threading.Thread(
		target=background_execute,
		name="thread: " + ' '.join(command),
		args=(command, reporting_job))
	t.start()

	with _rlock:
		_thread_list.append(t)


def wait_thread(job, timeout, cb, cbe):
	# We need to put the wait on it's own thread, so that we don't block the
	# entire dbus queue processing thread
	try:
		cb(job.state.Wait(timeout))
	except Exception as e:
		cbe("Wait exception: %s" % str(e))
	return 0


def add_wait(job, timeout, cb, cbe):

	if timeout == 0:
		# Users are basically polling, do not create thread
		cb(job.Complete)
	else:
		t = threading.Thread(
			target=wait_thread,
			name="thread job.Wait: %s" % job.dbus_object_path(),
			args=(job, timeout, cb, cbe)
		)

		t.start()
		with _rlock:
			_thread_list.append(t)
