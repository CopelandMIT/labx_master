classdef ifx_Avian_Test_Signal_Generator_Mode < int32
    %IFX_AVIAN_TEST_SIGNAL_GENERATOR_MODE  his type enumerates the
    %available modes of the Avian device's test signal generator.
    enumeration
	    IFX_TEST_SIGNAL_MODE_OFF               		 (0)  % The generator is not used
	    IFX_TEST_SIGNAL_MODE_BASEBAND_TEST           (1)  % The test signal is routed to the input of the baseband filters instead of the RX mixer output signals
	    IFX_TEST_SIGNAL_MODE_TOGGLE_TX_ENABLE        (2)  % TX1 is toggled on and off controlled by the generated test signal
        IFX_TEST_SIGNAL_MODE_TOGGLE_DAC_VALUE        (3)  % The power DAC value is toggled between minimum value and the value set through ifx_avian_cw_set_tx_dac_value controlled by the generated test signal.
		IFX_TEST_SIGNAL_MODE_TOGGLE_RX_SELF_TEST     (4)  % The test signal is routed directly into the RF mixer of RX antenna 1
    end
end
