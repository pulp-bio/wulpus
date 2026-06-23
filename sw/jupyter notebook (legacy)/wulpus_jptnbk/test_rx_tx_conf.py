import pytest
import numpy as np
import types

from wulpus_jptnbk.rx_tx_conf import WulpusRxTxConfigGen, TX_RX_MAX_NUM_OF_CONFIGS, MAX_CH_ID, RX_MAP, TX_MAP


def bits_set_positions(value: int):
    return [i for i in range(16) if (value >> i) & 1]


def test_add_single_tx_single_rx():
    gen = WulpusRxTxConfigGen()
    gen.add_config([0], [1])
    tx = gen.get_tx_configs()
    rx = gen.get_rx_configs()
    assert len(tx) == 1 and len(rx) == 1
    # TX channel 0 -> TX_MAP[0]
    assert bits_set_positions(tx[0]) == [TX_MAP[0]]
    # RX channel 1 -> RX_MAP[1]
    assert bits_set_positions(rx[0]) == [RX_MAP[1]]


def test_add_multiple_channels():
    gen = WulpusRxTxConfigGen()
    gen.add_config([0, 2, 4], [1, 3])
    tx_bits = bits_set_positions(gen.get_tx_configs()[0])
    rx_bits = bits_set_positions(gen.get_rx_configs()[0])
    assert sorted(tx_bits) == sorted(TX_MAP[[0, 2, 4]].tolist())
    assert sorted(rx_bits) == sorted(RX_MAP[[1, 3]].tolist())


def test_empty_lists():
    gen = WulpusRxTxConfigGen()
    gen.add_config([], [])
    assert gen.get_tx_configs()[0] == 0
    assert gen.get_rx_configs()[0] == 0


def test_invalid_channel_high():
    gen = WulpusRxTxConfigGen()
    with pytest.raises(ValueError):
        gen.add_config([MAX_CH_ID+1], [])
    with pytest.raises(ValueError):
        gen.add_config([], [MAX_CH_ID+2])


def test_invalid_channel_negative():
    gen = WulpusRxTxConfigGen()
    with pytest.raises(ValueError):
        gen.add_config([-1], [])
    with pytest.raises(ValueError):
        gen.add_config([], [-2])


def test_max_number_of_configs():
    gen = WulpusRxTxConfigGen()
    for _ in range(TX_RX_MAX_NUM_OF_CONFIGS):
        gen.add_config([0], [0])
    assert len(gen.get_tx_configs()) == TX_RX_MAX_NUM_OF_CONFIGS
    with pytest.raises(ValueError):
        gen.add_config([0], [0])


def test_length_counters_increase():
    gen = WulpusRxTxConfigGen()
    for i in range(5):
        gen.add_config([i % (MAX_CH_ID+1)], [(i+1) % (MAX_CH_ID+1)])
        assert gen.tx_rx_len == i+1


def test_optimized_switching_intersection_more_than_rx_only():
    gen = WulpusRxTxConfigGen()
    # tx & rx share two channels (0,1); rx only one (2)
    gen.add_config([0, 1, 3], [0, 1, 2], optimized_switching=True)
    tx_val = gen.get_tx_configs()[0]
    # Expect RX channels in intersect (0,1) added to TX config
    for ch in [0, 1]:
        assert (tx_val & (1 << RX_MAP[ch])) != 0


def test_optimized_switching_rx_only_more_than_intersection():
    gen = WulpusRxTxConfigGen()
    # intersect = {0}; rx_only = {1,2}
    gen.add_config([0, 3], [0, 1, 2], optimized_switching=True)
    tx_val = gen.get_tx_configs()[0]
    # Expect rx_only channels 1,2 pre-enabled in TX config
    for ch in [1, 2]:
        assert (tx_val & (1 << RX_MAP[ch])) != 0


def test_no_optimized_switching_changes():
    gen = WulpusRxTxConfigGen()
    gen.add_config([0, 2], [1, 3], optimized_switching=False)
    tx_val = gen.get_tx_configs()[0]
    # Ensure no RX bits accidentally set
    assert all((tx_val & (1 << RX_MAP[ch])) == 0 for ch in [1, 3])


def test_get_tx_rx_configs_slices():
    gen = WulpusRxTxConfigGen()
    gen.add_config([0], [0])
    gen.add_config([1], [1])
    tx = gen.get_tx_configs()
    rx = gen.get_rx_configs()
    assert tx.shape[0] == 2 and rx.shape[0] == 2
    # Internal arrays larger than length should remain zero outside slice
    assert np.all(gen.tx_configs[2:] == 0)
    assert np.all(gen.rx_configs[2:] == 0)


def test_repeated_add_same_config():
    gen = WulpusRxTxConfigGen()
    gen.add_config([0], [0])
    first_tx = gen.get_tx_configs()[0]
    gen.add_config([0], [0])
    second_tx = gen.get_tx_configs()[1]
    assert first_tx == second_tx


def test_mixed_zero_and_valid_channels():
    gen = WulpusRxTxConfigGen()
    gen.add_config([0, MAX_CH_ID], [0, MAX_CH_ID])
    tx_bits = bits_set_positions(gen.get_tx_configs()[0])
    rx_bits = bits_set_positions(gen.get_rx_configs()[0])
    assert TX_MAP[0] in tx_bits and TX_MAP[MAX_CH_ID] in tx_bits
    assert RX_MAP[0] in rx_bits and RX_MAP[MAX_CH_ID] in rx_bits


def test_build_tx_rx_configs_from_wulpus_config_basic():
    from wulpus_jptnbk.rx_tx_conf import build_tx_rx_configs_from_wulpus_config
    cfg_entry = types.SimpleNamespace(tx_channels=[0, 2], rx_channels=[
                                      1, 3], optimized_switching=False)
    wcfg = types.SimpleNamespace(tx_rx_config=[cfg_entry])
    tx, rx = build_tx_rx_configs_from_wulpus_config(wcfg)
    assert tx.shape == (1,) and rx.shape == (1,)
    assert (tx[0] & (1 << TX_MAP[0])) != 0
    assert (tx[0] & (1 << TX_MAP[2])) != 0
    assert (rx[0] & (1 << RX_MAP[1])) != 0
    assert (rx[0] & (1 << RX_MAP[3])) != 0


def test_build_tx_rx_configs_optimized_intersection():
    from wulpus_jptnbk.rx_tx_conf import build_tx_rx_configs_from_wulpus_config
    # intersection channels 0,1 > rx_only [2]
    cfg_entry = types.SimpleNamespace(tx_channels=[0, 1, 4], rx_channels=[
                                      0, 1, 2], optimized_switching=True)
    wcfg = types.SimpleNamespace(tx_rx_config=[cfg_entry])
    tx, rx = build_tx_rx_configs_from_wulpus_config(wcfg)
    # expect RX_MAP[0] and RX_MAP[1] bits added into TX
    for ch in [0, 1]:
        assert (tx[0] & (1 << RX_MAP[ch])) != 0


def test_build_tx_rx_configs_optimized_rx_only():
    from wulpus_jptnbk.rx_tx_conf import build_tx_rx_configs_from_wulpus_config
    # intersection channel 0, rx_only 1,2
    cfg_entry = types.SimpleNamespace(tx_channels=[0, 4], rx_channels=[
                                      0, 1, 2], optimized_switching=True)
    wcfg = types.SimpleNamespace(tx_rx_config=[cfg_entry])
    tx, rx = build_tx_rx_configs_from_wulpus_config(wcfg)
    for ch in [1, 2]:
        assert (tx[0] & (1 << RX_MAP[ch])) != 0


def test_build_tx_rx_configs_multiple_entries():
    from wulpus_jptnbk.rx_tx_conf import build_tx_rx_configs_from_wulpus_config
    entries = [
        types.SimpleNamespace(tx_channels=[0], rx_channels=[
                              0], optimized_switching=False),
        types.SimpleNamespace(tx_channels=[1, 2], rx_channels=[
                              3], optimized_switching=False),
    ]
    wcfg = types.SimpleNamespace(tx_rx_config=entries)
    tx, rx = build_tx_rx_configs_from_wulpus_config(wcfg)
    assert tx.shape == (2,) and rx.shape == (2,)
    assert (tx[0] & (1 << TX_MAP[0])) != 0
    assert (rx[1] & (1 << RX_MAP[3])) != 0


def test_build_tx_rx_configs_channel_validation():
    from wulpus_jptnbk.rx_tx_conf import build_tx_rx_configs_from_wulpus_config
    bad_entry = types.SimpleNamespace(
        tx_channels=[MAX_CH_ID+1], rx_channels=[], optimized_switching=False)
    wcfg = types.SimpleNamespace(tx_rx_config=[bad_entry])
    import pytest
    with pytest.raises(ValueError):
        build_tx_rx_configs_from_wulpus_config(wcfg)
