#include <gtest/gtest.h>
extern "C" {
#include "rfsimulator.h"
void exit_function(const char *file, const char *function, const int line, const char *s, const int assert)
{
  if (assert) {
    abort();
  }
  exit(EXIT_SUCCESS);
}
configmodule_interface_t *uniqCfg;
void *get_shlibmodule_fptr(char *modname, char *fname)
{
  return nullptr;
}
#include "openair1/SIMULATION/TOOLS/sim.h"
#include "nr_common.h"
extern int32_t signal_energy_nodc(int32_t *input,uint32_t length);
}
#include <algorithm>
#include <cmath>

channel_desc_t *get_awgn_channel_106_rb(void)
{
  double sample_rate;
  unsigned int samples_per_frame;
  double tx_bw;
  double rx_bw;
  double DS_TDL = .03;
  get_samplerate_and_bw(1, 106, 0, &sample_rate, &samples_per_frame, &tx_bw, &rx_bw);
  auto channel = new_channel_desc_scm(1, 1, AWGN, sample_rate / 1e6, 0, tx_bw, DS_TDL, 0.0, CORR_LEVEL_LOW, 0, 0, 0, -40);
  random_channel(channel, 0);
  return channel;
}

const int32_t rfsim_0dbm_sample_ref = 256;
const int32_t rfsim_k_value = rfsim_0dbm_sample_ref * rfsim_0dbm_sample_ref * 2;

TEST(rfsimulator, add_rx_input)
{
  int size = 12 * 50;
  std::vector<c16_t> input;
  input.resize(size);
  std::fill(input.begin(), input.end(), (c16_t){rfsim_0dbm_sample_ref, rfsim_0dbm_sample_ref});
  std::vector<c16_t> output;
  output.resize(size);
  std::fill(output.begin(), output.end(), (c16_t){0, 0});
  int rxAnt = 0;

  channel_desc_t *channel = get_awgn_channel_106_rb();
  int nbSamples = input.size();
  uint64_t TS = 0;
  uint32_t CirSize = sizeof(input);
  int16_t rx_gain = 0;
  int16_t tx_power = 0;
  rxAddInput(input.data(), output.data(), rxAnt, channel, nbSamples, TS, CirSize, rx_gain, tx_power);

  int32_t input_power = signal_energy_nodc((int32_t*)input.data(), input.size());
  int32_t output_power = signal_energy_nodc((int32_t*)output.data(), output.size());
  std::cout << "Input : " << 10*log10(input_power) - 10*log10(rfsim_k_value) << " [dBm] " << std::endl;
  std::cout << "Output : " <<  10*log10(output_power) - 10*log10(rfsim_k_value) << " [dBm]" << std::endl;

  channel->path_loss_dB = -3;
  random_channel(channel, 0);
  std::fill(output.begin(), output.end(), (c16_t){0, 0});

  rxAddInput(input.data(), output.data(), rxAnt, channel, nbSamples, TS, CirSize, rx_gain, tx_power);
  int32_t attenuated_power = signal_energy_nodc((int32_t*)output.data(), output.size());
  std::cout << "Output (3dB loss): " << 10*log10(attenuated_power) - 10*log10(rfsim_k_value) << " [dBm]" << std::endl;

  channel->path_loss_dB = 3;
  random_channel(channel, 0);
  std::fill(output.begin(), output.end(), (c16_t){0, 0});

  rxAddInput(input.data(), output.data(), rxAnt, channel, nbSamples, TS, CirSize, rx_gain, tx_power);
  int32_t increased_power = signal_energy_nodc((int32_t*)output.data(), output.size());
  std::cout << "Output (3dB gain): " << 10*log10(increased_power) - 10*log10(rfsim_k_value) << " [dBm]" << std::endl;

  channel->path_loss_dB = 3;
  std::fill(output.begin(), output.end(), (c16_t){0, 0});

  rxAddInput(input.data(), output.data(), rxAnt, channel, nbSamples, TS, CirSize, rx_gain, tx_power);
  increased_power = signal_energy_nodc((int32_t*)output.data(), output.size());
  std::cout << "Output (3dB gain) [no channel regeneration]: " << 10*log10(increased_power) - 10*log10(rfsim_k_value) << " [dBm]" << std::endl;

  channel->path_loss_dB = 0;
  std::fill(output.begin(), output.end(), (c16_t){0, 0});

  rxAddInput(input.data(), output.data(), rxAnt, channel, nbSamples, TS, CirSize, rx_gain, 3);
  increased_power = signal_energy_nodc((int32_t*)output.data(), output.size());
  std::cout << "Output (3dB gain) [increased tx power]: " << 10*log10(increased_power) - 10*log10(rfsim_k_value) << " [dBm]" << std::endl;

  channel->path_loss_dB = 0;
  std::fill(output.begin(), output.end(), (c16_t){0, 0});

  rxAddInput(input.data(), output.data(), rxAnt, channel, nbSamples, TS, CirSize, 3, tx_power);
  increased_power = signal_energy_nodc((int32_t*)output.data(), output.size());
  std::cout << "Output (3dB gain) [increased rx gain]: " << 10*log10(increased_power) - 10*log10(rfsim_k_value) << " [dBm]" << std::endl;

  channel->path_loss_dB = 0;
  channel->noise_power_dB = -20;
  std::fill(output.begin(), output.end(), (c16_t){0, 0});
  std::fill(input.begin(), input.end(), (c16_t){0, 0});
  rxAddInput(input.data(), output.data(), rxAnt, channel, nbSamples, TS, CirSize, rx_gain, tx_power);
  increased_power = signal_energy_nodc((int32_t*)output.data(), output.size());
  std::cout << "Output (0dB gain, " << channel->noise_power_dB << "dBm noise): " << 10*log10(increased_power) - 10*log10(rfsim_k_value) << " [dBm]" << std::endl;

  std::fill(input.begin(), input.end(), (c16_t){0, 0});
  std::fill(output.begin(), output.end(), (c16_t){0, 0});
  rxAddInput(input.data(), output.data(), rxAnt, channel, nbSamples, TS, CirSize, 3, tx_power);
  increased_power = signal_energy_nodc((int32_t*)output.data(), output.size());
  std::cout << "Output (3dB gain, " << channel->noise_power_dB << "dBm noise): " << 10*log10(increased_power) - 10*log10(rfsim_k_value) << " [dBm]" << std::endl;

  channel->path_loss_dB = 0;
  channel->noise_power_dB = -30;
  random_channel(channel, 0);
  auto linear_gain = std::pow(10,3/20.0);
  int16_t sample = rfsim_0dbm_sample_ref * linear_gain;
  std::fill(input.begin(), input.end(), (c16_t){sample, sample});
  std::fill(output.begin(), output.end(), (c16_t){0, 0});

  rxAddInput(input.data(), output.data(), rxAnt, channel, nbSamples, TS, CirSize, rx_gain, tx_power);
  increased_power = signal_energy_nodc((int32_t*)output.data(), output.size());
  std::cout << "Output (3dB) [digital sample gain]: " << 10*log10(increased_power) - 10*log10(rfsim_k_value) << " [dBm]" << std::endl;
}

int main(int argc, char **argv)
{
  logInit();
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
