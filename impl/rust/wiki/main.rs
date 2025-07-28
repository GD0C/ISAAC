use std::num::Wrapping as Wrap;

const MSG: &str = "Hello, World!";
const KEY: &str = "Secret";

fn main() {
    let mut isaac = Isaac::default();
    isaac.seed(KEY, true);
    let encrypted = isaac.vernam(MSG.as_bytes());

    println!("msg: {}", MSG);
    println!("Key: {}", KEY);
    print!("XOR encrypted: ");
    for a in &encrypted {
        print!("{:02x}", *a);
    }

    let mut isaac = Isaac::new();
    isaac.seed(KEY, true);
    let decrypted = isaac.vernam(&encrypted[..]);

    print!("\nXOR dcrypted: ");
    println!("{}", String::from_utf8(decrypted).unwrap())
}

macro_rules! mix_v(
   ($a:expr) => ({
       $a[0] ^= $a[1] << 11; $a[3] += $a[0]; $a[1] += $a[2];
       $a[1] ^= $a[2] >> 2;  $a[4] += $a[1]; $a[2] += $a[3];
       $a[2] ^= $a[3] << 8;  $a[5] += $a[2]; $a[3] += $a[4];
       $a[3] ^= $a[4] >> 16; $a[6] += $a[3]; $a[4] += $a[5];
       $a[4] ^= $a[5] << 10; $a[7] += $a[4]; $a[5] += $a[6];
       $a[5] ^= $a[6] >> 4;  $a[0] += $a[5]; $a[6] += $a[7];
       $a[6] ^= $a[7] << 8;  $a[1] += $a[6]; $a[7] += $a[0];
       $a[7] ^= $a[0] >> 9;  $a[2] += $a[7]; $a[0] += $a[1];
   });
);
/// ISAAC means Indirection, Shift, Accumulate, Add, and Count.
struct Isaac {
    mm: [Wrap<u32>; 256],
    aa: Wrap<u32>,
    bb: Wrap<u32>,
    cc: Wrap<u32>,
    rand_rsl: [Wrap<u32>; 256],
    rand_cnt: u32,
}

impl Isaac {
    fn new() -> Isaac {
        Isaac {
            mm: [Wrap(0); 256],
            aa: Wrap(0),
            bb: Wrap(0),
            cc: Wrap(0),
            rand_rsl: [Wrap(0); 256],
            rand_cnt: 0,
        }
    }
    fn isaac(&mut self) {
        self.cc += Wrap(1);
        self.bb += self.cc; 

        for i in 0..256 {
            let Wrap(x) = self.mm[i];
            match i % 4 {
                0 => self.aa ^= self.aa << 13,
                1 => self.aa ^= self.aa >> 6,
                2 => self.aa ^= self.aa << 2,
                3 => self.aa ^= self.aa >> 16,
                _ => unreachable!(),
            }

            self.aa += self.mm[((i + 128) % 256) as usize];
            let Wrap(y) = self.mm[((x >> 2) % 256) as usize] + self.aa + self.bb;
            self.mm[i] = Wrap(y);
            self.bb = self.mm[((y >> 10) % 256) as usize] + Wrap(x);
            self.rand_rsl[i] = self.bb;
        }
        self.rand_cnt = 0;
    }
    fn rand_init(&mut self, flag: bool) {
        let mut a_v = [Wrap(0x9e3779b9u32); 8];
            for _ in 0..4 {
                mix_v!(a_v);
            }
        for i in (0..256).step_by(8) {
            if flag {
                for (j, value) in a_v.iter_mut().enumerate().take(8) {
                    *value += self.rand_rsl[i + j];
            }
        }
            mix_v!(a_v);
            for (j, value) in a_v.iter().enumerate().take(8) {
                self.mm[i + j] = *value;
            }
        }
        if flag {
            for i in (0..256).step_by(8) {
                for (j, value) in a_v.iter_mut().enumerate().take(8) {
                    *value += self.mm[i + j];
                }
                mix_v!(a_v);
                for (j, value) in a_v.iter().enumerate().take(8) {
                    self.mm[i + j] = *value;
                }
            }
        }
        self.isaac();
        self.rand_cnt = 0;
    }
    fn i_random(&mut self) -> u32 {
        let r = self.rand_rsl[self.rand_cnt as usize];
        self.rand_cnt += 1;
        if self.rand_cnt > 255 {
            self.isaac();
            self.rand_cnt = 0;
        }
        r.0
    }

    fn seed(&mut self, seed: &str, flag: bool) {
        for i in 0..256 {
            self.mm[i] = Wrap(0);
        }
        for i in 0..256 {
            self.rand_rsl[i] = Wrap(0);
        }
        for i in 0..seed.len() {
            self.rand_rsl[i] = Wrap(u32::from(seed.as_bytes()[i]));
        }
        self.rand_init(flag);
    }
    fn i_rand_ascii(&mut self) -> u8 {
        (self.i_random() % 95 + 32) as u8
    }

    fn vernam(&mut self, msg: &[u8]) -> Vec<u8> {
        msg.iter().map(|&b| (self.i_rand_ascii() ^ b)).collect::<Vec<u8>>()
    }
}

impl Default for Isaac {
    fn default() -> Self {
        Isaac::new()
    }
}
