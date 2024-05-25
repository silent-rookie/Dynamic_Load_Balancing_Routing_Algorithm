import exputil
import sys
local_shell = exputil.LocalShell()


def test_distributed_exactly_equal(base_run_folder, multi_core_run_folder, num_cores):

    for f in [
        "pingmesh.csv",
        "tcp_flows.csv",
        "udp_bursts_incoming.csv",
        "udp_bursts_outgoing.csv",
        "link_utilization.csv",
        "link_queue_pkt.csv",
        "link_queue_byte.csv",
    ]:

        # Exit with error if the file does not exist
        if not local_shell.file_exists("%s/logs_ns3/%s" % (base_run_folder, f)):
            print("Missing: " + "%s/logs_ns3/%s" % (base_run_folder, f))
            exit(1)

        # Base lines
        base_lines = []
        with open("%s/logs_ns3/%s" % (base_run_folder, f)) as f_in:
            for line in f_in:
                base_lines.append(line.strip())

        # The merged lines from the various cores
        merged_lines = []
        for i in range(num_cores):
            with open("%s/logs_ns3/system_%d_%s" % (multi_core_run_folder, i, f)) as f_in:
                for line in f_in:
                    merged_lines.append(line.strip())

        # Sort the merged lines by the first three comma-separated integers
        def first_three_numbers_in_tuple(csv_line):
            spl = csv_line.split(",")
            return int(spl[0]), int(spl[1]), int(spl[2])
        merged_lines = sorted(merged_lines, key=first_three_numbers_in_tuple)

        # Line numbers must match up
        if len(base_lines) != len(merged_lines):
            raise ValueError("Line numbers do not match up")

        # Lines must exactly match up
        for i in range(len(base_lines)):
            if base_lines[i] != merged_lines[i]:
                raise ValueError("%s\n\n... is different from...\n\n%s" % (base_lines[i], merged_lines[i]))


def main():
    args = sys.argv[1:]
    if len(args) != 3:
        print("Must supply exactly three arguments")
        print("Usage: python test_distributed_exactly_equal.py [base_run_folder] [multi_core_run_folder] [num_cores]")
        exit(1)
    else:
        test_distributed_exactly_equal(
            args[0],
            args[1],
            int(args[2])
        )


if __name__ == "__main__":
    main()
