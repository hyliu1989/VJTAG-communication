import typing
from . import _wrapper

class VirtualJTAGDevice:
    """
    Handles the communication with the Virtual JTAG instances on an Altera FPGA.

    Note that the "jtag device" mentioned in the attributes means devices other than Altera FPGA, not the Virtual JTAG
    instances that is not being communicated with. This class communicates with all Virtual JTAG devices, though it is
    one at a time.

    Currently the codes assumes there is ONLY ONE jtag device, which is the FPGA that hosts the Virtual JTAG(s).

    Args:
        TODO

    Attributes:
        num_of_other_jtag_devices_before (int): Number of other "before" jtag devices. "Before" means those receiving
            TDI earlier than the VJTAG jtag device. The same applies to "after" devices where they receive TDI later
            than the VJTAG jtag device.
        num_of_other_jtag_devices_after (int): Number of other "after" jtag devices.
        len_of_irs_of_other_jtag_devices_before (int): The sum of the IR lengths of the other "before" jtag devices.
        len_of_irs_of_other_jtag_devices_after (int): The sum of the IR lengths of the other "after" jtag devices.
    """
    def __init__(
        self,
        rtl_report_file: str = None,
        vjtag_ir_widths: [int, typing.List[int]] = 1,
        vjtag_addrs: [int, typing.List[int]] = 0x10,
        vjtag_user1_lens: [int, typing.List[int]] = 5,
    ):
    if rtl_report_file is None:
        self._ir_widths = [vjtag_ir_widths] if type(vjtag_ir_widths) is int else [i for i in vjtag_ir_widths]
        self._addrs = [vjtag_addrs] if type(vjtag_addrs) is int else [i for i in vjtag_addrs]
        self._user1_lens = [vjtag_user1_lens] if type(vjtag_user1_lens) is int else [i for i in vjtag_user1_lens]
        if not (len(self._ir_widths) == len(self._addrs) == len(self._user1_lens)):
            raise ValueError("The vjtag parameters must have the same length (which equals to the number of devices.)")
    else:
        raise NotImplementedError
    self._n_vjtag_device = len(self._ir_widths)

    self.num_of_other_jtag_devices_before = 0
    self.num_of_other_jtag_devices_after = 0
    self.len_of_irs_of_other_jtag_devices_before = 0
    self.len_of_irs_of_other_jtag_devices_after = 0

    def reset(self):
        pass

    def count_number_of_jtag_devices(self):
        pass

    def send_vir(self, ir, vjtag_device=0):
        if vjtag_device >= self._n_vjtag_device:
            raise ValueError("VJTAG device id out of range.")
        pass

    def send_vdr(self, dr, bits, vjtag_device=0):
        if vjtag_device >= self._n_vjtag_device:
            raise ValueError("VJTAG device id out of range.")
        pass
